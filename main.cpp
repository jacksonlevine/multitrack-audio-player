#include "crow.h"
#include <string>
#include "sodium.h"
#include "jackspvars/pvarslib.hpp"

#include <map>
#include <sstream>

std::string load_file(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        return std::string("noexist");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

std::string url_decode(std::string text) {
    char h;
    std::ostringstream escaped;
    escaped.fill('0');

    for (auto i = text.begin(), n = text.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        if (c == '%') {
            if (i[1] && i[2]) {
                h = from_hex(i[1]) << 4 | from_hex(i[2]);
                escaped << h;
                i += 2;
            }
        } else if (c == '+') {
            escaped << ' ';
        } else {
            escaped << c;
        }
    }

    return escaped.str();
}

std::map<std::string, std::string> parse_form_data(const std::string& body) {
    std::map<std::string, std::string> form_data;
    std::stringstream ss(body);
    std::string key_value_pair;
    while (std::getline(ss, key_value_pair, '&')) {
        size_t equals_pos = key_value_pair.find('=');
        if (equals_pos != std::string::npos) {
            std::string key = url_decode(key_value_pair.substr(0, equals_pos));
            std::string value = url_decode(key_value_pair.substr(equals_pos + 1));
            form_data[key] = value;
        }
    }
    return form_data;
}

std::string generate_session_id(size_t length) {
    std::vector<unsigned char> buffer(length);
    randombytes_buf(buffer.data(), length);

    std::stringstream ss;
    for(auto& c : buffer) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }

    return ss.str();
}

std::string get_session_id(const crow::request& req)
{
     std::string cookie = req.get_header_value("Cookie");

        std::size_t pos = cookie.find("session_id=");
        if (pos != std::string::npos) {
            std::string session_id = cookie.substr(pos + 11);//skip to after session_id=

            return session_id;
        }
        return std::string("");
}

std::string get_extra_version(const char* view_name)
{
    std::string f = load_file((std::string("./views/") + view_name) + "_extra.html");
    if(f != "noexist")
    {
        return load_file((std::string("./views/") + view_name) + ".html") + f;
    }
    return load_file((std::string("./views/") + view_name) + ".html");
}

std::string load_file_or_extra_version(const char* view_name, const crow::request& req)
{
    std::string session_id = get_session_id(req);
        
        if(get_db_variable("sessions", session_id.c_str()).value() != "noexist")
        {
            return get_extra_version(view_name);
        }
        
        return load_file((std::string("./views/") + view_name) + ".html");
}

std::string load_file_or_extra_version_with_session_id(const char* view_name, std::string session_id)
{
        
        if(get_db_variable("sessions", session_id.c_str()).value() != "noexist")
        {
            return get_extra_version(view_name);
        }
        
        return load_file((std::string("./views/") + view_name) + ".html");
}

int main()
{

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return load_file("index.html");
    });


    CROW_ROUTE(app, "/tracks/<string>")
    ([](const crow::request& req, std::string filename){
        crow::response response;

        std::ifstream file("./static/tracks/" + filename, std::ios::binary);
        if (file) {
            response.body = std::string((std::istreambuf_iterator<char>(file)),
                                         std::istreambuf_iterator<char>());
            response.add_header("Content-Type", "audio/mpeg");
            return response;
        } else {
            response.code = 404;
            response.body = "File not found";
            return response;
        }
    });



    CROW_ROUTE(app, "/home")([](const crow::request& req){
        return load_file_or_extra_version("home", req);
    });

    CROW_ROUTE(app, "/register/form")([](){
        return load_file("./views/register.html");
    });

    CROW_ROUTE(app, "/login/form")([](){
        return load_file("./views/login.html");
    });

    CROW_ROUTE(app, "/register/submit").methods("POST"_method)(
        [](const crow::request& req){

            auto form_data = parse_form_data(req.body);
            std::string username = form_data["username"];
            std::string password = form_data["password"];
            std::string pconfirm = form_data["passwordConfirm"];

            if(password != pconfirm)
            {
                load_file("./views/register.html") + 
                "<br><p>The password and confirmation password do not match.</p>";
            }
            char hashed_password[crypto_pwhash_STRBYTES];

            //init sodium for hashing password
            if (sodium_init() < 0) {
                return crow::response(std::string("Sodium could not be loaded. Please try again."));
            }

            if (crypto_pwhash_str(
                    hashed_password, 
                    password.c_str(), 
                    password.size(),
                    crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                    crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
                return crow::response(std::string("Error hashing password. Please try again."));
            }
            
            if(get_db_variable("users", username.c_str()).value() == "noexist")
            {

                set_db_variable("users", username.c_str(), hashed_password);

            } else {

                return crow::response(load_file("./views/register.html") + 
                "<br><p>That username is already in use. Please use a different username.</p>");
            }
            std::string session_id = generate_session_id(16);
            set_db_variable("sessions", session_id.c_str(), "");
            crow::response res;
            res.set_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
            res.body = load_file_or_extra_version_with_session_id("home", session_id);
            return res;
        }
    );

    CROW_ROUTE(app, "/login/submit").methods("POST"_method)(
        [](const crow::request& req){

            auto form_data = parse_form_data(req.body);
            std::string username = form_data["username"];
            std::string password = form_data["password"];

            char hashed_password[crypto_pwhash_STRBYTES];

            //init sodium for hashing password
            if (sodium_init() < 0) {
                return crow::response(std::string("Sodium could not be loaded. Please try again."));
            }

            if (crypto_pwhash_str(
                    hashed_password, 
                    password.c_str(), 
                    password.size(),
                    crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                    crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
                return crow::response(std::string("Error hashing password. Please try again."));
            }

            bool authenticated = false;
            
            if(get_db_variable("users", username.c_str()).value() == "noexist")
            {
                return crow::response(load_file("./views/login.html") + 
                "<br><p>That username is not in our system. Please register first!</p>");


            } else {
                std::string stored_hash = get_db_variable("users", username.c_str()).value();

                if(crypto_pwhash_str_verify(hashed_password, password.c_str(), password.size()) == 0)
                {
                    authenticated = true;
                } else {
                    return crow::response(load_file("./views/login.html") + 
                "<br><p>The password you entered was incorrect.</p>");
                }
            }
            if(authenticated == true)
            {
                std::string session_id = generate_session_id(16);
                set_db_variable("sessions", session_id.c_str(), "");
                std::cout << "Session you just created: ";
                std::cout << session_id << std::endl;


                crow::response res;
                res.set_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
                res.body = load_file_or_extra_version_with_session_id("home", session_id);
                return res;
            } 
        }
    );

    CROW_ROUTE(app, "/logout").methods("POST"_method)([](const crow::request& req) {

        std::string session_id = get_session_id(req);

        if(get_db_variable("sessions", session_id.c_str()).value() != "noexist")
        {
            delete_db_variable("sessions", session_id.c_str());
            std::cout << "Deleting session ID" << std::endl;
        }
        crow::response res;
        res.set_header("Set-Cookie", "session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
        res.body = load_file("./views/home.html");
        res.code = 200;
        return res;
    });

    CROW_ROUTE(app, "/downloads")([](const crow::request& req){

        return load_file_or_extra_version("downloads", req);
    });

    app.port(8080).multithreaded().run();
}