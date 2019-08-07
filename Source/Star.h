//
//  Star.h
//  Area51
//
//  Created by Kevin Luu on 2019-08-06.
//  Copyright © 2019 Concordia. All rights reserved.
//


class Star
{
public:
    Star();
    Star(int texture_id);
    ~Star();
    
    static Star* GetInstance();
    
    void Draw();
    
private:
    void generateStars();
    void setTextureRotation(int shaderProgram, float textureRotationAngle);
};


