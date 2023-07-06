# template-video-player
## Provides a easy to understand working template for integrating video playback in your SDL application
Built with SDL, ffmpeg and imgui, combining easy to understand and fundamental tools
## For whom is it intended?
If you are new to video decoding and want to build an application with video playback (e.g. games) this template can indeed help you to get a basic video player in your app.
## How it works
1) The decoder first demuxes and than decodes the audio from the video and saves it locally as a file
2) The app reads in the decoded audio file and play it back
3) The app gets each frame decoded to the specified format (RGBA) from the video_reader
## Things left to do (up to you)
- Integrate a syncing mechanism, so that audio is always in sync with video. Since there are many different approaches, it's up to you.
- Playback controls
## Requirements
Only Cmake is requiered, since vcpk is bundled as a submodule, all other dependencies will automatically installed.
## Q&A
- Why does it takes so long to build? \
First time building requires some time, since ffmpeg takes a lot time to build.