const audio = document.getElementById('audio-player');
if (Hls.isSupported()) {
        const hls = new Hls();
        hls.loadSource('/hls/stream.m3u8');
        hls.attachMedia(audio);
        hls.on(Hls.Events.MANIFEST_PARSED, function () {
                audio.play();
        });
} else if (audio.canPlayType('application/vnd.apple.mpegurl')) {
        audio.src = '/hls/stream.m3u8';
        audio.addEventListener('loadedmetadata', function () {
                audio.play();
        });
}