#include "themes/glass/Theme.hpp"

namespace glass {

namespace Theme {

QFont titleFont()
{
    QFont font;
    font.setPointSizeF(8.5);
    font.setBold(true);
    font.setCapitalization(QFont::AllUppercase);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    return font;
}

} // namespace Theme

} // namespace glass
