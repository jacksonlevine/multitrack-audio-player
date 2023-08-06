let audioContext = new AudioContext();
let startTime = 0;  // To store the time when the audio started.
let pauseAt = 0;    // To store how much of the audio has been played before pausing.

let tracks = [
  "/tracks/t1.mp3", 
  "/tracks/t2.mp3", 
  "/tracks/t3.mp3",
  "/tracks/t4.mp3",
  "/tracks/t5.mp3"
];

let buffers = [];
let sources = [];
let isAudioPlaying = false;
let tracksLoaded = false;

let muted = [];
let soloed = [];

let mutehandle = (e) => {
  if(tracksLoaded)
  {
    let index = e.target.id.split("_")[0];
    
    if(muted[index] === false)
    {
      sources.forEach(source => {
        if(source.index === index)
        {
          source.source.stop();
        }
      })
      muted[index] = true;
    } else {
        let source = audioContext.createBufferSource();
        let offset = audioContext.currentTime - startTime;
        source.buffer = buffers[index];
        source.connect(audioContext.destination);
        source.start(startTime, offset);
        sources.push({source, index});
      muted[index] = false;
        
    }
  }
}
let solohandle = (e) => {
  if(tracksLoaded)
  {
    let index = e.target.id.split("_")[0];
    
    if(soloed[index] === false)
    {
      sources.forEach((source) => {
        if(source.index != index) {
          source.source.stop();
        }
      })
      for(let i = 0; i < muted.length; i++)
      {
        if(i !== index)
        {
          muted[i] = true;
        } else {
          muted[i] = false;
        }
      }
      for(let i = 0; i < soloed.length; i++)
      {
        if(i !== index)
        {
          soloed[i] = false;
        } else {
          soloed[i] = true;
        }
      }
      soloed[index] = true;
    } else {
      soloed[index] = false;
      sources.forEach(source => {
        source.source.stop();
      })
      sources = buffers.map((buffer, ind) => {


          let source = audioContext.createBufferSource();
          source.buffer = buffer;
          source.connect(audioContext.destination);
          let offset = audioContext.currentTime - startTime;
          source.start(startTime, offset);
          return {source, index: ind};

      });
    }
    
  }
}

onload = ()=>{

  let promises = tracks.map((track, index) => {
    return fetch(track)
      .then(response => response.arrayBuffer())
      .then(arrayBuffer => audioContext.decodeAudioData(arrayBuffer))
      .then(audioBuffer => {
        buffers[index] = audioBuffer;
      });
  });
  let trackControlDiv = document.getElementById("trackControls")

  let newGroup = document.createElement("div");

  tracks.map((track, index) => {
    let newDiv = document.createElement("div");
    newDiv.setAttribute("class", "horizontal");

    let button1 = document.createElement("button");
    button1.innerText = "Mute";
    button1.setAttribute("id", index + "_mute");
    button1.onclick = mutehandle;

    let button2 = document.createElement("button");
    button2.innerText = "Solo";
    button2.setAttribute("id", index + "_solo");
    button2.onclick = solohandle;

    newDiv.appendChild(button1);
    newDiv.appendChild(button2);

    let p = document.createElement("p");
    p.innerText = track;

    newDiv.appendChild(p);

    newGroup.appendChild(newDiv)

    muted.push(false);
    soloed.push(false);
  })

  trackControlDiv.innerHTML = "";
  trackControlDiv.appendChild(newGroup);


  Promise.all(promises).then(() => {
    document.getElementById("loading").setAttribute("class", "hidden");
    tracksLoaded = true;
  });

  document.getElementById("play")
    .addEventListener("click", ()=>{
      if(!isAudioPlaying)
      {
        if(audioContext.state === 'suspended') {
          audioContext.resume();
        }

        let startTime = audioContext.currentTime + 0.1;
        let offset = pauseAt;
        sources = buffers.map((buffer, index) => {
          if(muted[index] === false)
          {
            let source = audioContext.createBufferSource();
            source.buffer = buffer;
            source.connect(audioContext.destination);
            source.start(startTime, offset);
            return {source, index};
          }
        });
        startTime = audioContext.currentTime;

        isAudioPlaying = true;

      } else {
        let stopTime = audioContext.currentTime + 0.1;

        sources.forEach(source => {
          source.source.stop(stopTime);
        });
        pauseAt += audioContext.currentTime - startTime;
        
        isAudioPlaying = false;
      }
    })
}