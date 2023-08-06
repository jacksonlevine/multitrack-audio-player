audioContext = new AudioContext();
tracksLoaded = false;

class Track {
    constructor(buffer)
    {
        this.buffer = buffer;
        this.source = "";

        this.muted = false;
        this.soloed = false;

    }

    start(delayedStart, offset)
    {
        if(!this.muted){
        this.source = audioContext.createBufferSource();
        this.source.buffer = this.buffer;
        this.source.connect(audioContext.destination);

        this.source.start(delayedStart, offset+(delayedStart-audioContext.currentTime));
        }
    }

    stop(delayedStop)
    {
        
            this.source.stop(delayedStop);
        

    }


}

class MultiTrackPlayer
{
    constructor(songName)
    {
        this.songName = songName;
        this.paused = true;
        this.startTime = 0;
        this.pausedAt = 0;
        this.tracks = [];
    }

    fetchTracks()
    {
        tracksLoaded = false;
        return fetch("/tracks/" + this.songName + "/list")
            .then(response => response.json())
            .then(data => {


                let promises = data.tracks.map((track) => {
                    return fetch(`/tracks/${this.songName}/${track}`)
                      .then(response => response.arrayBuffer())
                      .then(arrayBuffer => audioContext.decodeAudioData(arrayBuffer))
                      .then(audioBuffer => {
                        let trk = new Track(audioBuffer);
                        this.tracks.push(trk);
                      });
                });
                Promise.all(promises).then(() => {
                    document.getElementById("loading").setAttribute("class", "hidden");
                    console.log(this.tracks)
                    tracksLoaded = true;
                    this.createUI("trackControls");
                });

            })
    }

    mute(index)
    {
        this.tracks[index].stop(audioContext.currentTime+0.1);
        this.tracks[index].muted = true;
        this.tracks[index].soloed = false;

        for(let i of this.tracks)
        {
            if(i.soloed)
            {
                i.soloed = false;
            }
        }
    }

    unmute(index)
    {
        this.tracks[index].muted = false;
        this.tracks[index].start(audioContext.currentTime, (this.pausedAt + audioContext.currentTime - this.startTime))
    }

    play()
    {
        if(audioContext.state === 'suspended') {
            audioContext.resume();
        }

        this.startTime = audioContext.currentTime;
        this.paused = false;
        let del = audioContext.currentTime + 0.1;
        for(let i of this.tracks)
        {
            i.start(del, this.pausedAt);
        }
    }

    pause()
    {
        this.paused = true;
        this.pausedAt += audioContext.currentTime - this.startTime;

        let del = audioContext.currentTime + 0.1;
        for(let i of this.tracks)
        {
            i.stop(del);
        }
    }

    play_or_pause()
    {
        if(this.paused)
        {
            this.play();
        } else {
            this.pause();
        }
    }

    mutehandle = (e) =>
    {
        let index = e.target.id.split("_")[0];
        if(!this.tracks[index].muted)
        {
            this.mute(index);
        } else {
            this.unmute(index);
        }
            
    }
    solohandle = (e) =>
    {
        let index = e.target.id.split("_")[0];
        if(!this.tracks[index].soloed)
        {
            this.tracks.forEach((track, ind)=> {
                if(ind != index)
                {
                    this.mute(ind);
                }
            })
            this.tracks[index].soloed = true;
        } else {
            this.tracks.forEach((track, ind)=> {
                if(ind != index)
                {
                    this.unmute(ind);
                }

            })
            this.tracks[index].soloed = false;
        }
        
    }

    createUI(divId)
    {
        let trackControlDiv = document.getElementById(divId)

        let newGroup = document.createElement("div");
        let index = 0;
        for(let i of this.tracks){
            let newDiv = document.createElement("div");
            newDiv.setAttribute("class", "horizontal");

            let button1 = document.createElement("button");
            button1.innerText = "Mute";
            button1.setAttribute("id", index + "_mute");


            let button2 = document.createElement("button");
            button2.innerText = "Solo";
            button2.setAttribute("id", index + "_solo");


            button1.addEventListener("click", (e) => this.mutehandle(e));
            button2.addEventListener("click", (e) => this.solohandle(e));

            newDiv.appendChild(button1);
            newDiv.appendChild(button2);

            let p = document.createElement("p");
            p.innerText = this.songName + " " + index;

            newDiv.appendChild(p);

            newGroup.appendChild(newDiv)
            index++;
        }

        let newDiv = document.createElement("div");
        newDiv.setAttribute("class", "horizontal");

        let button1 = document.createElement("button");
        button1.setAttribute("id", "play");
        button1.innerText = "Play/Pause";
        button1.addEventListener("click", () => this.play_or_pause());

        newDiv.appendChild(button1);


        trackControlDiv.innerHTML = "";
        trackControlDiv.appendChild(newGroup);
        trackControlDiv.appendChild(newDiv);
    }
}

onload = () => {
    mult = new MultiTrackPlayer("pocoloco");
    mult.fetchTracks();

    
}