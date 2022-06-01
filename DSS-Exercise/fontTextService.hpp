//
//  fontTextService.hpp
//  DSS-Exercise
//
//  Created by Benjamin Lee on 11/16/19.
//  Copyright © 2019 Benjamin Lee. All rights reserved.
//

#ifndef fontTextService_hpp
#define fontTextService_hpp

#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include "types.h"
#include "textureService.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

class FontTextService {
public:
    enum class Font : uint32_t { Roboto20 = 0, Roboto22 = 1, Roboto36 = 2, Roboto48 = 3 };
    
    FontTextService() = delete;
    FontTextService(const std::shared_ptr<TextureService>& textureService, bool verbose);
    ~FontTextService();
    
    std::shared_ptr<Texture> getString(const std::string& name);
    std::shared_ptr<Texture> addString(Font font, const std::string& name, const std::string& str, const Color& color, uint32_t wrapLength = 0);
    void removeString(const std::string& name);

private:
    bool                            verbose_;
    SDL_Renderer                    *renderer_;
    std::shared_ptr<TextureService> textureService_;
    std::vector<TTF_Font *>         fonts_;
    
    // Admittedly this may look odd. Since I am using SDL_ttf, that lib treats strings as textures.
    // Typical systems would use freetype to create textures as well as track glyph data for each character
    // And the render them on the fly, perhaps even utilizing VBO/IBO
    // In this case, I've separated out and tracked the textures for strings here, as a means of indicating
    // The special nature of this as well as a workable "abstraction" for handling text based on the limitations of the rendering system
    std::mutex                                                  mutex_;
    std::unordered_map<std::string, std::shared_ptr<Texture>>   strings_;
};

#endif /* fontTextService_hpp */
