const audio = document.getElementById('audio-player');
const dash = dashjs.MediaPlayer().create();
dash.initialize(audio, "/dash/stream.mpd", true);