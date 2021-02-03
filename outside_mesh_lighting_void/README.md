#TODO & Progress
##Goal-0
Engine must load outside sourced models and textures
##Done
Example models and textures added to repo
Names and paths changed to compile locally
Shaders render models with ambient light only
##Goal-1
Engine must load models that reflect spectral light
##Done
Shader names refactored to show individual models
Vertex Shaders now track the position and value of spectral light source
Fragment Shaders calculate texture color and add that value to environmental lighting
Shaders render models with spectral and ambient light
