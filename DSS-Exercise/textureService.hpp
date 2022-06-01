//
//  textureService.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef textureService_hpp
#define textureService_hpp

#include <stdio.h>
#include "texture.hpp"
#include "errors.hpp"
#include "resourceFetcherService.hpp"
#include <SDL2/SDL.h>

#include <memory>
#include <functional>
#include <mutex>
#include <unordered_map>

class TextureService {
public:
    TextureService() = delete;
    TextureService(SDL_Renderer *renderer, const std::shared_ptr<ResourceFetcherService>& fetcher, bool verbose);
    ~TextureService();
    
    void createTexture(const std::string& name, const std::string& url, std::function<void(Error, std::shared_ptr<Texture>)> callback);
    std::shared_ptr<Texture> createTexture(const std::string& name, SDL_Surface *surface, bool destroySurface);

    std::shared_ptr<Texture> getTexture(const std::string& name);
    void removeTexture(const std::string& name);
    void removeTexture(const std::shared_ptr<Texture>& texture);
    
private:
    bool                                    verbose_;
    SDL_Renderer                            *renderer_;
    std::shared_ptr<ResourceFetcherService> fetcher_;
    
    std::mutex                              mutex_;
    std::unordered_map<std::string, std::shared_ptr<Texture>>   textures_;
};

#endif /* textureService_hpp */
