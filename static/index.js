
const createMultiTrack = (songName) => {


  const fetchTrackList = () =>
  {

      return fetch("/tracks/" + songName + "/list")
          .then(response => response.json())
          .then(data => {
              console.log(data.tracks);
              return data.tracks;
          })
  }

  fetchTrackList().then((trackList)=> {

    let groupOfTracks = [];
    let index = 0;
    for(let i of trackList)
    {
      let internalObj = {
        id: index,
        draggable: false,
        startPosition: 0,
        volume: 1,
        url: `/tracks/${songName}/${i}`
      };
      groupOfTracks.push(internalObj);
      index += 1;
      
    }

    return groupOfTracks;

  }).then(groupOfTracks => {

    const multitrack = Multitrack.create(
      groupOfTracks,
      {
        container: document.querySelector('#multitrack'), // required!
        minPxPerSec: 10, // zoom level
        rightButtonDrag: true, // drag tracks with the right mouse button
        cursorWidth: 2,
        cursorColor: '#D72F21',
        trackBackground: '#2D2D2D',
        trackBorderColor: '#7C7C7C',
        envelopeOptions: {
          lineColor: 'rgba(255, 0, 0, 0.7)',
          lineWidth: 4,
          dragPointSize: 8,
          dragPointFill: 'rgba(255, 255, 255, 0.8)',
          dragPointStroke: 'rgba(255, 255, 255, 0.3)',
        },
      },
    )

    
    
    return multitrack;

  }).then(multitrack => {

    // Play/pause button
    const button = document.querySelector('#play')
    button.disabled = true
    multitrack.once('canplay', () => {
      button.disabled = false
      button.onclick = () => {
        multitrack.isPlaying() ? multitrack.pause() : multitrack.play()
        button.textContent = multitrack.isPlaying() ? 'Pause' : 'Play'
      }
    })
    
    // Forward/back buttons
    const forward = document.querySelector('#forward')
    forward.onclick = () => {
      multitrack.setTime(multitrack.getCurrentTime() + 30)
    }
    const backward = document.querySelector('#backward')
    backward.onclick = () => {
      multitrack.setTime(multitrack.getCurrentTime() - 30)
    }
    
    // Zoom
    const slider = document.querySelector('input[type="range"]')
    slider.oninput = () => {
      multitrack.zoom(slider.valueAsNumber)
    }


    
    // Destroy the plugin on unmount
    // This should be called before calling initMultiTrack again to properly clean up
    window.onbeforeunload = () => {
      multitrack.destroy()
    }

  });

}


onload = () => createMultiTrack("pocoloco");