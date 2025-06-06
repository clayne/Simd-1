/*
* Tests for Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2025 Yermalayeu Ihar,
*               2014-2016 Antonenka Mikhail.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Test/TestCompare.h"
#include "Test/TestPerformance.h"
#include "Test/TestString.h"
#include "Test/TestRandom.h"
#include "Test/TestFile.h"
#include "Test/TestOptions.h"

#ifdef SIMD_OPENCV_ENABLE
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif

namespace Test
{
    namespace
    {
        struct FuncO
        {
            typedef void(*FuncPtr)(const uint8_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride);
            FuncPtr func;
            String description;

            FuncO(const FuncPtr & f, const String & d) : func(f), description(d) {}

            void Call(const View & src, View & dst) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.width, src.height, src.stride, dst.data, dst.stride);
            }
        };

        struct FuncN
        {
            typedef void(*FuncPtr)(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride);
            FuncPtr func;
            String description;

            FuncN(const FuncPtr& f, const String& d) : func(f), description(d) {}

            void Call(const View& src, View& dst) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.stride, src.width, src.height, dst.data, dst.stride);
            }
        };
    }

#define FUNC_O(func) FuncO(func, #func)

#define FUNC_N(func) FuncN(func, #func)

    template<class Func> bool AnyToAnyAutoTest(int width, int height, View::Format srcType, View::Format dstType, const Func & f1, const Func & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " for size [" << width << "," << height << "].");

        View src(width, height, srcType, NULL, TEST_ALIGN(width));
        FillRandom(src);
        //FillSequence(src);

        View dst1(width, height, dstType, NULL, TEST_ALIGN(width));
        View dst2(width, height, dstType, NULL, TEST_ALIGN(width));

        Simd::Fill(dst1, 1);
        Simd::Fill(dst2, 2);

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, dst1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, dst2));

        result = result && Compare(dst1, dst2, 0, true, 64);

        return result;
    }

    template<class Func> bool AnyToAnyAutoTest(View::Format srcType, View::Format dstType, const Func & f1, const Func & f2)
    {
        bool result = true;

        result = result && AnyToAnyAutoTest(W, H, srcType, dstType, f1, f2);
        result = result && AnyToAnyAutoTest(W + O, H - O, srcType, dstType, f1, f2);

        return result;
    }

    bool BgraToBgrAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Bgr24, FUNC_O(Simd::Base::BgraToBgr), FUNC_O(SimdBgraToBgr));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Bgr24, FUNC_O(Simd::Sse41::BgraToBgr), FUNC_O(SimdBgraToBgr));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::F)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Bgr24, FUNC_O(Simd::Avx2::BgraToBgr), FUNC_O(SimdBgraToBgr));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Bgr24, FUNC_O(Simd::Avx512bw::BgraToBgr), FUNC_O(SimdBgraToBgr));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Bgr24, FUNC_O(Simd::Neon::BgraToBgr), FUNC_O(SimdBgraToBgr));
#endif 

        return result;
    }

    bool BgraToGrayAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Gray8, FUNC_O(Simd::Base::BgraToGray), FUNC_O(SimdBgraToGray));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Gray8, FUNC_O(Simd::Sse41::BgraToGray), FUNC_O(SimdBgraToGray));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Gray8, FUNC_O(Simd::Avx2::BgraToGray), FUNC_O(SimdBgraToGray));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Gray8, FUNC_O(Simd::Avx512bw::BgraToGray), FUNC_O(SimdBgraToGray));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Gray8, FUNC_O(Simd::Neon::BgraToGray), FUNC_O(SimdBgraToGray));
#endif 

        return result;
    }

    bool BgraToRgbAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgb24, FUNC_O(Simd::Base::BgraToRgb), FUNC_O(SimdBgraToRgb));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgb24, FUNC_O(Simd::Sse41::BgraToRgb), FUNC_O(SimdBgraToRgb));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::F)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgb24, FUNC_O(Simd::Avx2::BgraToRgb), FUNC_O(SimdBgraToRgb));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgb24, FUNC_O(Simd::Avx512bw::BgraToRgb), FUNC_O(SimdBgraToRgb));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgb24, FUNC_O(Simd::Neon::BgraToRgb), FUNC_O(SimdBgraToRgb));
#endif

        return result;
    }

    bool BgraToRgbaAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgba32, FUNC_O(Simd::Base::BgraToRgba), FUNC_O(SimdBgraToRgba));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgba32, FUNC_O(Simd::Sse41::BgraToRgba), FUNC_O(SimdBgraToRgba));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgba32, FUNC_O(Simd::Avx2::BgraToRgba), FUNC_O(SimdBgraToRgba));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgba32, FUNC_O(Simd::Avx512bw::BgraToRgba), FUNC_O(SimdBgraToRgba));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Bgra32, View::Rgba32, FUNC_O(Simd::Neon::BgraToRgba), FUNC_O(SimdBgraToRgba));
#endif

        return result;
    }

    bool BgrToGrayAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Gray8, FUNC_O(Simd::Base::BgrToGray), FUNC_O(SimdBgrToGray));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Gray8, FUNC_O(Simd::Sse41::BgrToGray), FUNC_O(SimdBgrToGray));
#endif 

#if defined(SIMD_AVX2_ENABLE) && !defined(SIMD_CLANG_AVX2_BGR_TO_BGRA_ERROR)
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Gray8, FUNC_O(Simd::Avx2::BgrToGray), FUNC_O(SimdBgrToGray));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Gray8, FUNC_O(Simd::Avx512bw::BgrToGray), FUNC_O(SimdBgrToGray));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Gray8, FUNC_O(Simd::Neon::BgrToGray), FUNC_O(SimdBgrToGray));
#endif

        return result;
    }

    bool BgrToHslAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Hsl24, FUNC_O(Simd::Base::BgrToHsl), FUNC_O(SimdBgrToHsl));

        return result;
    }

    bool BgrToHsvAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Hsv24, FUNC_O(Simd::Base::BgrToHsv), FUNC_O(SimdBgrToHsv));

        return result;
    }

    bool BgrToLabAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Lab24, FUNC_N(Simd::Base::BgrToLab), FUNC_N(SimdBgrToLab));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Lab24, FUNC_N(Simd::Sse41::BgrToLab), FUNC_N(SimdBgrToLab));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Lab24, FUNC_N(Simd::Avx2::BgrToLab), FUNC_N(SimdBgrToLab));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Lab24, FUNC_N(Simd::Avx512bw::BgrToLab), FUNC_N(SimdBgrToLab));
#endif 

        return result;
    }

    bool BgrToRgbAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Rgb24, FUNC_O(Simd::Base::BgrToRgb), FUNC_O(SimdBgrToRgb));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Rgb24, FUNC_O(Simd::Sse41::BgrToRgb), FUNC_O(SimdBgrToRgb));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Rgb24, FUNC_O(Simd::Avx2::BgrToRgb), FUNC_O(SimdBgrToRgb));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Rgb24, FUNC_O(Simd::Avx512bw::BgrToRgb), FUNC_O(SimdBgrToRgb));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Bgr24, View::Rgb24, FUNC_O(Simd::Neon::BgrToRgb), FUNC_O(SimdBgrToRgb));
#endif

        return result;
    }

    bool GrayToBgrAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Gray8, View::Bgr24, FUNC_O(Simd::Base::GrayToBgr), FUNC_O(SimdGrayToBgr));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Gray8, View::Bgr24, FUNC_O(Simd::Sse41::GrayToBgr), FUNC_O(SimdGrayToBgr));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Gray8, View::Bgr24, FUNC_O(Simd::Avx2::GrayToBgr), FUNC_O(SimdGrayToBgr));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Gray8, View::Bgr24, FUNC_O(Simd::Avx512bw::GrayToBgr), FUNC_O(SimdGrayToBgr));
#endif

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Gray8, View::Bgr24, FUNC_O(Simd::Neon::GrayToBgr), FUNC_O(SimdGrayToBgr));
#endif 

        return result;
    }

    bool GrayToYAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Base::GrayToY), FUNC_N(SimdGrayToY));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Sse41::GrayToY), FUNC_N(SimdGrayToY));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Avx2::GrayToY), FUNC_N(SimdGrayToY));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Avx512bw::GrayToY), FUNC_N(SimdGrayToY));
#endif

//#ifdef SIMD_NEON_ENABLE
//        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
//            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Neon::GrayToY), FUNC_N(SimdGrayToY));
//#endif 

        return result;
    }

    bool Int16ToGrayAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Int16, View::Gray8, FUNC_O(Simd::Base::Int16ToGray), FUNC_O(SimdInt16ToGray));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Int16, View::Gray8, FUNC_O(Simd::Sse41::Int16ToGray), FUNC_O(SimdInt16ToGray));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Int16, View::Gray8, FUNC_O(Simd::Avx2::Int16ToGray), FUNC_O(SimdInt16ToGray));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Int16, View::Gray8, FUNC_O(Simd::Avx512bw::Int16ToGray), FUNC_O(SimdInt16ToGray));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Int16, View::Gray8, FUNC_O(Simd::Neon::Int16ToGray), FUNC_O(SimdInt16ToGray));
#endif 

        return result;
    }

    bool RgbToGrayAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Rgb24, View::Gray8, FUNC_O(Simd::Base::RgbToGray), FUNC_O(SimdRgbToGray));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Rgb24, View::Gray8, FUNC_O(Simd::Sse41::RgbToGray), FUNC_O(SimdRgbToGray));
#endif 

#if defined(SIMD_AVX2_ENABLE) && !defined(SIMD_CLANG_AVX2_BGR_TO_BGRA_ERROR)
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Rgb24, View::Gray8, FUNC_O(Simd::Avx2::RgbToGray), FUNC_O(SimdRgbToGray));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Rgb24, View::Gray8, FUNC_O(Simd::Avx512bw::RgbToGray), FUNC_O(SimdRgbToGray));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Rgb24, View::Gray8, FUNC_O(Simd::Neon::RgbToGray), FUNC_O(SimdRgbToGray));
#endif

        return result;
    }

    bool RgbaToGrayAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Rgba32, View::Gray8, FUNC_O(Simd::Base::RgbaToGray), FUNC_O(SimdRgbaToGray));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Rgba32, View::Gray8, FUNC_O(Simd::Sse41::RgbaToGray), FUNC_O(SimdRgbaToGray));
#endif 

#if defined(SIMD_AVX2_ENABLE)
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Rgba32, View::Gray8, FUNC_O(Simd::Avx2::RgbaToGray), FUNC_O(SimdRgbaToGray));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Rgba32, View::Gray8, FUNC_O(Simd::Avx512bw::RgbaToGray), FUNC_O(SimdRgbaToGray));
#endif 

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
            result = result && AnyToAnyAutoTest(View::Rgba32, View::Gray8, FUNC_O(Simd::Neon::RgbaToGray), FUNC_O(SimdRgbaToGray));
#endif

        return result;
    }

    bool YToGrayAutoTest()
    {
        bool result = true;

        if (TestBase())
            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Base::YToGray), FUNC_N(SimdYToGray));

#ifdef SIMD_SSE41_ENABLE
        if (Simd::Sse41::Enable && TestSse41() && W >= Simd::Sse41::A)
            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Sse41::YToGray), FUNC_N(SimdYToGray));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && TestAvx2() && W >= Simd::Avx2::A)
            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Avx2::YToGray), FUNC_N(SimdYToGray));
#endif 

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable && TestAvx512bw())
            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Avx512bw::YToGray), FUNC_N(SimdYToGray));
#endif

//#ifdef SIMD_NEON_ENABLE
//        if (Simd::Neon::Enable && TestNeon() && W >= Simd::Neon::A)
//            result = result && AnyToAnyAutoTest(View::Gray8, View::Gray8, FUNC_N(Simd::Neon::YToGray), FUNC_N(SimdYToGray));
//#endif 

        return result;
    }

    //-----------------------------------------------------------------------------------------------------

    bool ConvertImageSpecialTest(size_t width, size_t height, View::Format srcFormat, View::Format dstFormat)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test image conversion from " << ToString(srcFormat) << " to " << ToString(dstFormat) << " pixel format.");

        View src(width, height, srcFormat, NULL, TEST_ALIGN(width));
        FillRandom(src);

        View dst(width, height, dstFormat, NULL, TEST_ALIGN(width));

        Simd::Convert(src, dst);

        if (srcFormat != dstFormat && dstFormat != View::Gray8 && 
            (srcFormat != View::Bgra32 || dstFormat == View::Rgba32) &&
            (srcFormat != View::Rgba32 || dstFormat == View::Bgra32))
        {
            View rev(width, height, srcFormat, NULL, TEST_ALIGN(width));

            Simd::Convert(dst, rev);

            result = result && Compare(src, rev, 0, true, 64, 0);
        }

        return result;
    }

    bool ConvertImageSpecialTest(const Options & options)
    {
        bool result = true;

        View::Format formats[5] = { View::Gray8, View::Bgr24, View::Bgra32, View::Rgb24, View::Rgba32 };
        for (size_t s = 0; s < 5; ++s)
            for (size_t d = 0; d < 5; ++d)
                result = result && ConvertImageSpecialTest(W, H, formats[s], formats[d]);

        return result;
    }

    //-----------------------------------------------------------------------------------------------------

    bool BgrToLabSpecialTest(const Options & options)
    {
        bool result = true;
#ifdef SIMD_OPENCV_ENABLE
        TEST_LOG_SS(Info, "Test OpenCV and Simd image conversion from BGR to LAB pixel format.");

        View src(W, H, View::Bgr24, NULL, TEST_ALIGN(W));
        FillRandom(src);

        View dst1(W, H, View::Lab24, NULL, TEST_ALIGN(W));
        View dst2(W, H, View::Lab24, NULL, TEST_ALIGN(W));

        Simd::Fill(dst1, 1);
        Simd::Fill(dst2, 2);

        TEST_EXECUTE_AT_LEAST_MIN_TIME(TEST_PERFORMANCE_TEST("OpenCV"); cv::cvtColor((cv::Mat)src, (cv::Mat)(dst1.Ref()), cv::COLOR_BGR2Lab, 3));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(TEST_PERFORMANCE_TEST("Simd"); Simd::BgrToLab(src, dst2));

        result = result && Compare(dst1, dst2, 0, true, 64);

#ifdef TEST_PERFORMANCE_TEST_ENABLE
        TEST_LOG_SS(Info, PerformanceMeasurerStorage::s_storage.ConsoleReport(false, true));
        PerformanceMeasurerStorage::s_storage.Clear();
#endif

#endif
        return result;
    }

}
