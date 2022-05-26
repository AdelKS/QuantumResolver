### Quantum Resolver

This is an attempt at writing a faster Gentoo dependency resolver with a hope to get it upstreamed and replace that specific part in `emerge`. This project is at its early beginnings.

The current main ideas:
- Map package names, use flags, use expands to unique integers, for example `sys-devel/gcc` -> `13`, `pgo` -> `13` (use flags do not live in the same world as package names), and work only with integers while resolving dependencies.
  - For this purpose, various classes have been implemented (_e.g._ [NamedVector](src/named_vector.h), [MultiKeyMap](src/multikey_map.h) and [Bijection](src/bijection.h) that enable going back and forth between strings and integers.
- Parse the initial files on disk only once and produce a fast intermediate representation that helps speeding up dependency resolution. This applies to _e.g._ to `ebuild` versions (see [ebuild_version.h](src/ebuild_version.h)), dependencies and flags (see [ebuild.h](src/ebuild.h)).


#### Currently exposed feature to the CLI

One can query the state of the flags a package would be emerged with, basically this output from `emerge`:

```shell
> $ emerge -av ffmpeg
This action requires superuser access...
Would you like to add --pretend to options? [Yes/No] Yes

These are the packages that would be merged, in order:

Calculating dependencies... done!
[ebuild   R    ] media-video/ffmpeg-4.4.2-r1:0/56.58.58::gentoo-adel  USE="X alsa amf bzip2 dav1d encode fdk fontconfig gnutls gpl iconv jpeg2k ladspa libass lzma mp3 network openal opengl opus postproc pulseaudio rubberband sdl speex srt svg theora threads truetype v4l vaapi vdpau vmaf vorbis vpx vulkan webp x264 x265 xvid zlib -amr -amrenc (-appkit) -bluray -bs2b -cdio -chromaprint -chromium -codec2 -cpudetection -cuda -debug -doc -flite -frei0r -fribidi -gcrypt -gme -gmp -gsm -hardcoded-tables -iec61883 -ieee1394 -jack -kvazaar -libaom -libaribb24 -libcaca -libdrm -libilbc -librtmp -libsoxr -libtesseract -libv4l -libxml2 -lv2 (-mipsdspr1) (-mipsdspr2) (-mipsfpu) (-mmal) -modplug -nvenc -opencl -openh264 -openssl -oss -pic -rav1e -samba -snappy -sndio -ssh -static-libs -svt-av1 -test -twolame -verify-sig -vidstab -zeromq -zimg -zvbi" ABI_X86="32 (64) (-x32)" CPU_FLAGS_X86="aes avx avx2 fma3 mmx mmxext sse sse2 sse3 sse4_1 sse4_2 ssse3 -3dnow -3dnowext -fma4 -xop" FFTOOLS="aviocat cws2fws ffescape ffeval ffhash fourcc2pixfmt graph2dot ismindex pktdumper qt-faststart sidxindex trasher" 0 KiB
```

To obtain the same thing with `quantum`:

```shell
./quantum status "=media-video/ffmpeg-4.4.2"
```

which outputs something like this

```shell
######################################
media-video/ffmpeg   Version: 4.4.2
  USE="X alsa amf bzip2 dav1d encode fdk fontconfig gnutls gpl iconv jpeg2k ladspa libass lzma mp3 network openal opengl opus postproc pulseaudio rubberband sdl speex srt svg theora threads truetype v4l vaapi vdpau vmaf vorbis vpx vulkan webp x264 x265 xvid zlib -amr -amrenc -appkit -bluray -bs2b -cdio -chromaprint -chromium -codec2 -cpudetection -cuda -debug -doc -flite -frei0r -fribidi -gcrypt -gme -gmp -gsm -hardcoded-tables -iec61883 -ieee1394 -jack -kvazaar -libaom -libaribb24 -libcaca -libdrm -libilbc -librtmp -libsoxr -libtesseract -libv4l -libxml2 -lv2 -mipsdspr1 -mipsdspr2 -mipsfpu -mmal -modplug -nvenc -opencl -openh264 -openssl -oss -pic -rav1e -samba -snappy -sndio -ssh -static-libs -svt-av1 -test -twolame -verify-sig -vidstab -zeromq -zimg -zvbi "
  ABI_X86="32 64 "
  ARCH="amd64 "
  CPU_FLAGS_X86="aes avx avx2 fma3 mmx mmxext sse sse2 sse3 sse4_1 sse4_2 ssse3 "
  FFTOOLS="aviocat cws2fws ffescape ffeval ffhash fourcc2pixfmt graph2dot ismindex pktdumper qt-faststart sidxindex trasher "
```

If one uses a less strict package atom, _e.g._ simply `media-video/ffmpeg`, it will output the flag state for each version of the package
#### How to (e)build

In its current state, you need `qmake` (I will switch to `meson` when things get more serious)

```shell
qmake CONFIG+="release" QuantumResolver.pro
make
```

For a debug build

```shell
qmake CONFIG+='debug sanitizer sanitize_address' QuantumResolver.pro
make
```

Otherwise, if you have `QtCreator` you can simply open the `.pro` file and setup the project for "Release" and "Debug" builds. Then press the "Play" button.
