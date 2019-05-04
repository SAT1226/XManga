#ifndef __BLITZ_H
#define __BLITZ_H

/*
 Copyright (C) 1998, 1999, 2001, 2002, 2004, 2005, 2007
      Daniel M. Duley <daniel.duley@verizon.net>
 (C) 2004 Zack Rusin <zack@kde.org>
 (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
 (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
 (C) 1998, 1999 Dirk Mueller <mueller@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <QtGui/QImage>

class Blitz
{
public:
    enum GradientType {VerticalGradient=0, HorizontalGradient, DiagonalGradient,
        CrossDiagonalGradient, PyramidGradient, RectangleGradient,
        PipeCrossGradient, EllipticGradient};

    enum RGBChannel{Grayscale=0, Brightness, Red, Green, Blue, Alpha, All};
    enum EffectQuality{Low=0, High};

    enum ScaleFilterType{UndefinedFilter=0, PointFilter, BoxFilter,
        TriangleFilter, HermiteFilter, HanningFilter, HammingFilter,
        BlackmanFilter, GaussianFilter, QuadraticFilter, CubicFilter,
        CatromFilter, MitchellFilter, LanczosFilter, BesselFilter,
        SincFilter};

    enum ModulationType{Intensity=0, Saturation, HueShift, Contrast};

    /**
     * Smoothscales an image using a high-quality filter.
     *
     * @param img The image to smoothscale.
     * @param sz The size to scale to.
     * @param blur A blur factor. Values greater than 1.0 blur while values
     * less than 1.0 sharpen.
     * @param filter The filter type.
     * @param aspectRatio What aspect ratio to use, if any.
     * @return The processed image. The original is not changed.
     * @author Daniel M. Duley (mosfet)
     */
    static QImage smoothScaleFilter(QImage &img, const QSize &sz,
                                    float blur=1.0,
                                    ScaleFilterType filter=BlackmanFilter,
                                    Qt::AspectRatioMode aspectRatio =
                                    Qt::IgnoreAspectRatio);
    static QImage smoothScaleFilter(QImage &img, int dwX, int dwY,
                                    float blur=1.0,
                                    ScaleFilterType filter=BlackmanFilter,
                                    Qt::AspectRatioMode aspectRatio =
                                    Qt::IgnoreAspectRatio);
};

#endif
