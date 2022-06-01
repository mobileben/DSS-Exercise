//
//  main.cpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/15/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#include <iostream>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "curl/curl.h"
#include "openssl/crypto.h"

#include <args.hxx> // https://github.com/Taywee/args

#include "workerPool.h"
#include "epoch.h"
#include "rapidjson/document.h"
#include "utilities.hpp"

#include "feed.hpp"
#include "carousel.hpp"
#include "textureService.hpp"
#include "fontTextService.hpp"
#include "feedService.hpp"
#include "resourceFetcherService.hpp"
#include "carousel.hpp"
#include "dateSelector.hpp"
#include "input.hpp"
#include "uiOverlay.hpp"
#include "displayList.hpp"

#include <future>
#include <memory>
#include <functional>

// Non-static here just for convenience for usage in Carousel
extern const int32_t SCREEN_WIDTH = 1920;
extern const int32_t SCREEN_HEIGHT = 1080;

extern const int32_t DATE_SELECTOR_X = SCREEN_WIDTH / 2;
extern const int32_t DATE_SELECTOR_Y = 220;

extern const int ThumbnailWidth = 480;
extern const int ThumbnailHeight = 270;

extern const std::string kTextureKeyLeft = "key-left";
extern const std::string kTextureKeyRight = "key-right";
extern const std::string kTextureKeyUp = "key-up";
extern const std::string kTextureKeyDown = "key-down";
static const std::string kTextureKeyBkg = "bkg";

// Non-static here just for convenience for usage in Carousel
extern const std::string kTextureKeyLoadingIcon = "loading-icon";
extern const std::string kTextureKeyLinkError = "link-error";

static const std::string kTextureAssetLeft = "key-left.png";
static const std::string kTextureAssetRight = "key-right.png";
static const std::string kTextureAssetUp = "key-up.png";
static const std::string kTextureAssetDown = "key-down.png";
static const std::string kTextureAssetBkg = "1.jpg";
static const std::string kTextureAssetLoading = "loading.png";
static const std::string kTextureAssetLinkError = "link-error.png";

static const std::string kInitializingStringKey = "initializing";
static const std::string kInitializingStringValue = "Initializing...";
static const FontTextService::Font kInitializingStringFont = FontTextService::Font::Roboto48;
static const std::string kLoadingStringKey = "loading";
static const std::string kLoadingStringValue = "Loading...";
static const FontTextService::Font kLoadingStringFont = FontTextService::Font::Roboto48;

enum class DemoState { Uninitialized, Initializing, Ready };

/* we have this global to let the callback get easy access to it */
static pthread_mutex_t *lockarray;

static void lock_callback(int mode, int type, char *file, int line)
{
  (void)file;
  (void)line;
  if (mode & CRYPTO_LOCK) {
    pthread_mutex_lock(&(lockarray[type]));
  }
  else {
    pthread_mutex_unlock(&(lockarray[type]));
  }
}

static unsigned long thread_id(void)
{
  unsigned long ret;

  ret=(unsigned long)pthread_self();
  return(ret);
}

static void init_locks(void)
{
  int i;

  lockarray=(pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
                                        sizeof(pthread_mutex_t));
  for (i=0; i<CRYPTO_num_locks(); i++) {
    pthread_mutex_init(&(lockarray[i]),NULL);
  }

  CRYPTO_set_id_callback((unsigned long (*)())thread_id);
  CRYPTO_set_locking_callback((void (*)(int, int, const char*, int))lock_callback);
}

static void kill_locks(void)
{
  int i;

  CRYPTO_set_locking_callback(NULL);
  for (i=0; i<CRYPTO_num_locks(); i++)
    pthread_mutex_destroy(&(lockarray[i]));

  OPENSSL_free(lockarray);
}

std::string getCurrentWorkingDirectory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return std::string(cwd);
    } else {
        throw std::runtime_error("Could not get current working directory.");
    }
}

bool initializeMinimalBakedGoods(const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTextService, const std::shared_ptr<FeedService>& feedService, const std::string& cwd, bool verbose) {
    bool success = true;
    std::future<bool> future;
    std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();
    future = promise->get_future();
    
    std::string asset = "file://" + cwd + "/baked/" + kTextureAssetBkg;
    texService->createTexture(kTextureKeyBkg, asset, [asset, promise, verbose](Error error, std::shared_ptr<Texture> texture) {
        if (error != Error::None) {
            std::cerr << "ERROR: Could not create baked good " << asset << std::endl;
        } else if (verbose) {
            std::cout << "Created baked texture " << asset << std::endl;
        }
        promise->set_value(error == Error::None);
    });
    if (!fontTextService->addString(kInitializingStringFont, kInitializingStringKey, kInitializingStringValue, { 0xFF, 0xFF, 0xFF, 0xFF })) {
        std::cerr << "Could not create string '" << kInitializingStringValue << "'" << std::endl;
        return false;
    } else if (!fontTextService->addString(kLoadingStringFont, kLoadingStringKey, kLoadingStringValue, { 0xFF, 0xFF, 0xFF, 0xFF })) {
        std::cerr << "Could not create string '" << kLoadingStringValue << "'" << std::endl;
        return false;
    }
    if (!future.get()) {
        success = false;
    }
    return success;
}

std::vector<std::shared_future<bool>> initializeBakedGoods(const std::shared_ptr<TextureService>& texService, const std::shared_ptr<FontTextService>& fontTextService, const std::shared_ptr<FeedService>& feedService, const std::string& cwd, bool verbose) {
    const std::string left = "file://" + cwd + "/baked/key-left.png";
    const std::string bkg = "file://" + cwd + "/baked/1.jpg";
    std::vector<std::pair<std::string, std::string>> bakedTextures = {
        { kTextureKeyLeft, kTextureAssetLeft },
        { kTextureKeyRight, kTextureAssetRight },
        { kTextureKeyUp, kTextureAssetUp },
        { kTextureKeyDown, kTextureAssetDown },
        { kTextureKeyLoadingIcon, kTextureAssetLoading },
        { kTextureKeyLinkError, kTextureAssetLinkError }
    };

    std::vector<std::shared_future<bool>> futures;
    for (auto bt : bakedTextures) {
        std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();
        futures.push_back(promise->get_future());
        
        std::string asset = "file://" + cwd + "/baked/" + bt.second;
        texService->createTexture(bt.first, asset, [asset, promise, verbose](Error error, std::shared_ptr<Texture> texture) {
            promise->set_value(error == Error::None);
            if (error != Error::None) {
                std::cerr << "ERROR: Could not create baked good " << asset << std::endl;
            } else if (verbose) {
                std::cout << "Created baked texture " << asset << std::endl;
            }
        });
    }

    // Prime our initial feed
    std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();
    futures.push_back(promise->get_future());
    feedService->fetchFeed(feedService->getDefaultDate(), [promise, verbose](Error error, uint32_t status, std::shared_ptr<Feed> feed) {
        promise->set_value(error == Error::None);
        if (error != Error::None) {
            std::cerr << "ERROR: Could not fetch initial feed" << std::endl;
        } else if (verbose) {
            std::cout << "Fetched initial feed" << std::endl;
        }
    });
    
    return futures;
}

int main(int argc, const char * argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer *renderer = nullptr;
    std::shared_ptr<TextureService> texService;
    std::shared_ptr<FontTextService> fontTextService;
    std::shared_ptr<ResourceFetcherService> resourceFetcherService;
    std::shared_ptr<FeedService> feedService;
    std::string workingDirectory;
    std::string execFullname = argv[0];
    std::vector<std::string> parts;
    utilities::componentsSeparatedByDelimiter(execFullname, '/', parts);
    auto execName = parts.back();
    args::ArgumentParser parser(execName);
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Flag verboseFlag(parser, "verbose", "Verbose output", {"verbose"});
    args::ValueFlag<uint32_t> numWorkersArg(parser, "num_workers", "Number of Resource Fetcher Worker Threads", {"num_workers"});
    args::ValueFlag<uint32_t> stressArg(parser, "stress", "Number of seconds to sleep after network call to stress system", {"stress"});
    bool verbose = false;
    uint32_t numWorkers = 4;
    uint32_t stress = 0;

    // Parse arguments. Utilize separate try/catch to compartmentalize exception handling
    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help& e) {
        std::cout <<  parser;
        return 0;
    } catch (args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;

    } catch(args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 2;
    }

    try {
        if (args::get(verboseFlag)) {
            verbose = true;
        }
        if (numWorkersArg) {
            numWorkers = args::get(numWorkersArg);
            if (numWorkers == 0) {
                numWorkers = 1;
                std::cerr << "Trying to set number of workers to 0 ... forcing value to 1" << std::endl;
            } else if (verbose) {
                std::cout << "Setting number of worker threads to " << numWorkers << std::endl;
            }
        }
        if (stressArg) {
            stress = args::get(stressArg);
        }
        workingDirectory = getCurrentWorkingDirectory();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    OPENSSL_init();
    init_locks();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Could not initialize SDL2: " << SDL_GetError() << std::endl;
        return 1;
    }
    window = SDL_CreateWindow(
                  "DSS Exercise 1",
                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                  SCREEN_WIDTH, SCREEN_HEIGHT,
                  SDL_WINDOW_SHOWN
                  );
    if (window == nullptr) {
        std::cerr << "Could not create SDL_Window: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Could not create SDL_Renderer: " << SDL_GetError() << std::endl;
        return 1;
    }
 
    // Use Linear Filtering, since we are scaling down textures
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
    
    // Initializing services that rely on SDL being initialized
    try {
        resourceFetcherService = std::make_shared<ResourceFetcherService>(numWorkers, stress, verbose);
        texService = std::make_shared<TextureService>(renderer, resourceFetcherService, verbose);
        fontTextService = std::make_shared<FontTextService>(texService, verbose);
        feedService = std::make_shared<FeedService>(resourceFetcherService, texService, fontTextService, ThumbnailWidth, verbose);
    } catch (std::exception& e) {
        // TODO: Services will throw, need to throw on error (in particular fontTextService)
        std::cerr << "Exception creating services: " << e.what() << std::endl;
        return 1;
    }
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);

    // Initialized baked assets
    // We block until this is done
    if (!initializeMinimalBakedGoods(texService, fontTextService, feedService, workingDirectory, verbose)) {
        std::cerr << "Could not initialize " << execName << std::endl;
        std::cerr << "Is the `baked` directory in the same directory as " << execName << "?" << std::endl;
        return 1;
    }

    DisplayList *displaylist = new DisplayList(renderer, { 0, 0, 0, 0xFF});

    try {
        bool quit = false;
        SDL_Event e;
        DemoState state = DemoState::Uninitialized;
        DemoState nextState = DemoState::Initializing;
        int64_t lastTime = EpochTime::timeInMicroSec();
        std::shared_ptr<Texture> bkgTex = texService->getTexture(kTextureKeyBkg);
        std::shared_ptr<Texture> initializingText = fontTextService->getString(kInitializingStringKey);
        std::vector<std::shared_future<bool>> initializingFutures;
        
        std::shared_ptr<Carousel> carousel;
        std::shared_ptr<DateSelector> dateSelector;
        std::unique_ptr<UiOverlay> uiOverlay;
        
        while (!quit) {
            Input input;
            auto now = EpochTime::timeInMicroSec();
            double dt = static_cast<double>(now - lastTime) / 1000000.;
            lastTime = now;

            input.clear();
            
            while (SDL_PollEvent(&e)) {

                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                        input.left = true;
                    }
                    if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                        input.right = true;
                    }
                    if (e.key.keysym.scancode == SDL_SCANCODE_UP) {
                        input.up = true;
                    }
                    if (e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                        input.down = true;
                    }
                } else if (e.type == SDL_QUIT) {
                    quit = true;
                }
            }

            if (!quit) {
                displaylist->clear();
                displaylist->addTexture(bkgTex, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
                
                if (nextState != state) {
                    switch (nextState) {
                        case DemoState::Initializing: {
                            initializingFutures = initializeBakedGoods(texService, fontTextService, feedService, workingDirectory, verbose);
                        }
                            break;
                        case DemoState::Ready: {
                            carousel = std::make_shared<Carousel>(CarouselConfig{SCREEN_HEIGHT / 2, 0.66667, ThumbnailWidth, ThumbnailHeight, 32, 4, 4, static_cast<int>(ThumbnailWidth * 2.75), { 0x40, 0x40, 0x40, 0x90 }, { 0xFF, 0xFF, 0xFF, 0xFF }}, texService, fontTextService, verbose);
                            dateSelector = std::make_shared<DateSelector>(texService, fontTextService, feedService, carousel, DATE_SELECTOR_X, DATE_SELECTOR_Y, verbose);
                            uiOverlay.reset(new UiOverlay(texService, fontTextService, carousel, dateSelector, SCREEN_WIDTH, SCREEN_HEIGHT, stress, numWorkers));
                            auto feed = feedService->getFeed(feedService->getDefaultDate());
                            if (feed && carousel) {
                                carousel->setFeed(feed);
                            }
                        }
                            break;
                        default:
                            break;
                    }
                    state = nextState;
                } else {
                    switch (state) {
                        case DemoState::Initializing: {
                            size_t count = 0;
                            bool success = true;
                            for (auto& f : initializingFutures) {
                                const auto fs = f.wait_for(std::chrono::seconds(0));
                                if (fs == std::future_status::ready) {
                                    ++count;
                                }
                            }
                            
                            if (count == initializingFutures.size()) {
                                for (auto& f : initializingFutures) {
                                    bool res = f.get();
                                    if (!res) {
                                        success = false;
                                    }
                                }
                                
                                if (!success) {
                                    std::cerr << "Could not initialize " << execName << std::endl;
                                    return 1;
                                } else {
                                    nextState = DemoState::Ready;
                                    initializingFutures.clear();
                                }
                            }
                            // Display the initializing text
                            displaylist->addTexture(initializingText, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
                        }
                            break;
                        case DemoState::Ready: {
                            if (dateSelector) {
                                dateSelector->update(dt, displaylist, input);
                            }
                            if (carousel) {
                                carousel->update(dt, displaylist, input);
                            }
                            if (uiOverlay) {
                                uiOverlay->update(dt, displaylist);
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
                displaylist->render();
            }
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    
    delete displaylist;
    
    // Destroy our services in reverse order
    feedService = nullptr;
    fontTextService = nullptr;
    texService = nullptr;
    resourceFetcherService = nullptr;
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    kill_locks();
    
    return 0;
}
