#include "Memory.h"

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

class FontRenderer
{
  public:
    FontRenderer();

    FT_Library Library;
    FT_Face Face;
};

