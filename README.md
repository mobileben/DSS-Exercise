# Overview
This is the code base for my submission for the "DSS - NCP Engineer - Take Home Exercise v2". This toy assignment is on point in testing if someone could recreate (in an albeit simplified manner) the rough discovery UI for a streaming service.

My main goal of the assignment was to illustrate a rough viable working architecture that one may typically run into when implementing such as system. In particular, key elements of a system are creating a multi-threaded environment where resources (eg. feeds, assets) may be downloaded independent of main thread execution. Moreover, parts of the infrastructure also must know how to operate "independent" of said transactions. Less focus is spent on visual layout and visual effects. Due to time constraints, focus on the former was chosen.

Project was created using Xcode 11.2 (11B52) on an iMac Pro 10 core running OSX 10.14.6. There are 3 main directories in the project directory. `3rdParty`, `executable`, and `DSS-Exercise`. `3rdParty` contains all the built 3rd Party libs in use. They were either built using `./configure`, `git clone`'d (in the case of header only libs), or copied from my filesystem (`openssl`). `executable` contains the executable and a directory called `baked`. That directory is required to be in the same directory as the executable. As the executable run is coded to get the current working directory and then append `baked` and then any baked asset to the path. `DSS-Exercise` contains all the source code.

This root directory of this project contains this file, the `xcodeproj` file, and a working executable.

# How to Build
I used Xcode as the build tool. Note that I am familiar with CMake and have my own projects that utilize it to build the server side components. It did not seem practical to create a CMake system since.

I am expecting a reviewer that plans on building the project on the Mac to have knowledge in how to build an Xcode project. Note that `3rdParty` contains prebuilts. These are prebuilts I already had on my machine from other projects. They are static libs, so will be built into the project. This project was built on an iMac Pro (Intel).

# How to Run
The executable provide is the `Debug` configuration. While I did build the `Release` configuration, I haven't really tested it. This is a toy, so right now it isn't doing too much that warrants a `Release` configuration to test against.

```
$ DSS-Exercise
```

You can run 

```
$ DSS-Exercise -h
```

To see the various flags that can be used.

### -h, --help
This will call up the help.

### --verbose
This will output some information at runtime. Admittedly I had planned on outputting more information. As time progressed I had less time to focus on this. So it is very sparse at this point.

### --num_workers
The executable relies on worker threads for doing all network calls. This is based on a thread pool. This flag indicates the number of threads to use in the pool. The default is 4. If a value of 0 or lower is used, it will use 1 thread.

### --stress
One of the features of the executable is the ability to handle network calls in with a thread pool. The app can function with slow network. For example, loading status is shown in thumbnails that are in the process of being loaded. To help test/demonstrate this, this flag can be used. This is the number of seconds to sleep after a network call. This provides an easy means to "simulate" a slow network.

## Controls
The UI will show the keys you can use. In general they are the left key and right key to move the carousel and the up key and down key to change dates. Command+Q will quit.

# Design Decisions
The real core of this executable is the thread pool and how the rest of the program utilizes it. I prefer this type of approach versus say using `std::async` as I know just how how many threads are in use. 

I have chosen at times to use exception handling as a means of surfacing errors. There are certainly cases in the code where I am not doing it. This dual nature approach was primarily dictated by time constraints. Exception handling for error propogation generally requires a little bit more time and some experimentation to be consistently applied throughout a project.

I have not spent too much time worrying about performance or even memory footprint. While I realize the documentation cites lower end requirements, I see that comment as a bit of a red herring for this assignment. Architectures that are geared for low memory need to take in consider the overall needs, the design philosophy of the architecture, how much memory is available, and more importantly what the data is. This is a loosely specified problem (ie. no spec for the JSON format). Thus it was better to not focus on these types of issues.

I have chosen to use smart pointers. I've worked with both raw and smart pointers. In this case, they were chosen more for convenience. See the above paragraph.

I have chosen to pass in services needed to objects. There are numerous other approaches such as Singletons and globals. Most methods have some their fans as well as their detractors. I myself have worked with several different methods and personally don't care either way. Provided the solution is well thought through, it will more than likely be usable. I chose this approach just because it would be less controversial than say using a Singletons.

This is a curated experience. I have baked in dates you can go through. Url construction and for that matter request construction is severely dumbed down for this project.

SDL was chosen since it had easy to use font/text support (see Known Issues) and reduced to workload into creating my own rendering system. While it was relatively simple to get up and running, it has its share of limitations.

libcurl was chosen as a relatively barebones simple way to make networks calls. I have not used it previously, so it is possible there are some issues around implementation.

I chosen not to fully parse the JSON feed data. I did create some data structures around what I needed and even parsed some information that was not needed. This was to primarly demonstrate the code is capable of parsing JSON objects and arrays. Rapidjson is used primarily because my last project used it, so it was easy to implement.

I'm using a display list architecture for rendering. This is just one of many approaches. I've simplifed rendering by not supporting sort order (which normally should be done) as well a renderer that does not maintain render state (eg. blend modes, colors, etc). So it is a little wasteful.

I've forgone alot of commenting. I believe the code should be fairly readable. It is not to say the code is devoid of comments. There are some in key places. This was done to save time.

File naming may seem a bit odd and inconsistent. I opted for lower camelcase mainly because I started with some lower case file names and then adding UpperCamelCase variants looked a tad odd. Another quirk is the existence of `.hpp` and `.h` files. This is a byproduct of Xcode creating `.hpp` when creating a C++ file that has both a `.cpp` and header. But if you create solely a C++ header, it will name it `.h`. Needless to say, I'd just conform to what the project uses. The file naming in use is annoying enough to me I've mentioned it :D!

# Known Issues

Pathing internally uses `*nix` style pathing when loading baked goods.

While I put in some error handling, it is by no means complete. A good example of this is network failure. I do not gracefully handle things. Note however, I have stubbed in support for it. For example, if the thumbnail image failed to download, I do actually have a "broken link" icon that can be displayed and have code like this 

```
// We are either error or loading
if (thumb.recap->getThumbnailState() == FeedGameRecap::ThumbnailState::Loading) {
    displaylist->addXformTexture(loadingIconTex_, thumb.x, thumb.y, loadingRotate_, scale, scale, colorOp, grayed);
} else if (thumb.recap->getThumbnailState() == FeedGameRecap::ThumbnailState::Error) {
    displaylist->addScaledTexture(linkErrorTex_, thumb.x, thumb.y, scale, scale, colorOp, grayed);
}
```

However code like this has not been tested well.

The scrolling code has issues and IMHO is slightly atrocious. Of all things, this took a little more time than I had timeboxed. At the current pixel/sec scroll speed, it looks okay, but is flawed. I could have refined it but did not want to spend the extra time doing so as I passed my time box.

SDL text handling is terrrible. Their approach is to render to a texture. Additionally, when using wrapped text, it suffers from 2 issues: You cannot center the text, and it seems that when wrapping occurs, the resulting texture will be the wrap limit. This consequence of this is that you can have room to the right of empty space. This will be noticeble in the demo. I center all text to the thumbnail, but some wrapped text will look left aligned. This is a by product of the issues. Normal systems would render fonts and optimize by usage of things like VBO/IBO if necessary. I have done implementations of this, but I deem that out of scope.
