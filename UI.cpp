#include "UI.h"

#include <stdexcept>

FontRenderer::FontRenderer()
{
  FT_Init_FreeType(&Library);

  FT_New_Face(Library, FONTDIR"Golfah-Marika.otf", 0, &Face);


}

