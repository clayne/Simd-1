/*
* Tests for Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2025 Yermalayeu Ihar.
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
#ifndef __TestTensor_h__
#define __TestTensor_h__

#include "Test/TestLog.h"

namespace Test
{
    typedef std::vector<size_t> Index;

    //-------------------------------------------------------------------------------------------------

    template<class T> SimdTensorDataType DataType();

    template<> SIMD_INLINE SimdTensorDataType DataType<float>() { return SimdTensorData32f; };
    template<> SIMD_INLINE SimdTensorDataType DataType<uint16_t>() { return SimdTensorData16f; };
    template<> SIMD_INLINE SimdTensorDataType DataType<uint8_t>() { return SimdTensorData8u; };

    //-------------------------------------------------------------------------------------------------

    template<class T> class Tensor
    {
    public:
        typedef T Type;

        SIMD_INLINE Tensor()
            : _size(0)
            , _format(SimdTensorFormatUnknown)
        {
        }

        SIMD_INLINE Tensor(const Test::Shape & shape, SimdTensorFormatType format = SimdTensorFormatUnknown, const Type & value = Type())
            : _shape(shape)
            , _format(format)
        {
            Resize(value);
        }

        SIMD_INLINE Tensor(std::initializer_list<size_t> shape, SimdTensorFormatType format = SimdTensorFormatUnknown, const Type & value = Type())
            : _shape(shape.begin(), shape.end())
            , _format(format)
        {
            Resize(value);
        }

        SIMD_INLINE ~Tensor()
        {
        }

        SIMD_INLINE void Reshape(const Test::Shape & shape, SimdTensorFormatType format = SimdTensorFormatUnknown, const Type & value = Type())
        {
            _shape = shape;
            _format = format;
            Resize(value);
        }

        SIMD_INLINE void Reshape(std::initializer_list<size_t> shape, SimdTensorFormatType format = SimdTensorFormatUnknown, const Type & value = Type())
        {
            _shape.assign(shape.begin(), shape.end());
            _format = format;
            Resize(value);
        }

        SIMD_INLINE void Extend(const Test::Shape & shape, const Type& value = Type())
        {
            _shape = shape;
            Extend(value);
        }

        SIMD_INLINE void Extend(std::initializer_list<size_t> shape, const Type& value = Type())
        {
            _shape.assign(shape.begin(), shape.end());
            Extend(value);
        }

        SIMD_INLINE void Clone(const Tensor& tensor)
        {
            _shape = tensor._shape;
            _format = tensor._format;
            _size = tensor._size;
            _data = tensor._data;
        }

        static SIMD_INLINE SimdTensorDataType DataType()
        {
            return Test::DataType<Type>();
        }

        SIMD_INLINE SimdTensorFormatType Format() const
        {
            return _format;
        }

        SIMD_INLINE const Test::Shape & Shape() const
        {
            return _shape;
        }

        SIMD_INLINE size_t Count() const
        {
            return _shape.size();
        }

        SIMD_INLINE size_t Index(ptrdiff_t axis) const
        {
            if (axis < 0)
                axis += _shape.size();
            return axis;
        }

        SIMD_INLINE size_t Axis(ptrdiff_t axis) const
        {
            return _shape[Index(axis)];
        }

        SIMD_INLINE size_t Size(ptrdiff_t startAxis, ptrdiff_t endAxis) const
        {
            startAxis = Index(startAxis);
            endAxis = Index(endAxis);
            assert(startAxis <= endAxis && (size_t)endAxis <= _shape.size());
            size_t size = 1;
            for (ptrdiff_t axis = startAxis; axis < endAxis; ++axis)
                size *= _shape[axis];
            return size;
        }

        SIMD_INLINE size_t Size(ptrdiff_t startAxis) const
        {
            return Size(startAxis, _shape.size());
        }

        SIMD_INLINE size_t Size() const
        {
            return _size;
        }

        SIMD_INLINE size_t Offset(const Test::Index & index) const
        {
            assert(_shape.size() == index.size());

            size_t offset = 0;
            for (size_t axis = 0; axis < _shape.size(); ++axis)
            {
                assert(_shape[axis] > 0);
                assert(index[axis] < _shape[axis]);

                offset *= _shape[axis];
                offset += index[axis];
            }
            return offset;
        }

        SIMD_INLINE size_t Offset(std::initializer_list<size_t> index) const
        {
            assert(_shape.size() == index.size());

            size_t offset = 0;
            for (const size_t * s = _shape.data(), *i = index.begin(); i < index.end(); ++s, ++i)
            {
                assert(*s > 0);
                assert(*i < *s);

                offset *= *s;
                offset += *i;
            }
            return offset;
        }

        SIMD_INLINE Type * Data()
        {
            return _data.data();
        }

        SIMD_INLINE const Type * Data() const
        {
            return _data.data();
        }

        SIMD_INLINE Type * Data(const Test::Index & index)
        {
            return Data() + Offset(index);
        }

        SIMD_INLINE const Type * Data(const Test::Index & index) const
        {
            return Data() + Offset(index);
        }

        SIMD_INLINE Type * Data(std::initializer_list<size_t> index)
        {
            return Data() + Offset(index);
        }

        SIMD_INLINE const Type * Data(std::initializer_list<size_t> index) const
        {
            return Data() + Offset(index);
        }

        SIMD_INLINE size_t Batch() const
        {
            assert(_shape.size() == 4 && (_format == SimdTensorFormatNchw || _format == SimdTensorFormatNhwc));
            return _shape[0];
        }

        SIMD_INLINE size_t Channels() const
        {
            assert(_shape.size() == 4 && (_format == SimdTensorFormatNchw || _format == SimdTensorFormatNhwc));
            return _format == SimdTensorFormatNchw ? _shape[1] : _shape[3];
        }

        SIMD_INLINE size_t Height() const
        {
            assert(_shape.size() == 4 && (_format == SimdTensorFormatNchw || _format == SimdTensorFormatNhwc));
            return _format == SimdTensorFormatNchw ? _shape[2] : _shape[1];
        }

        SIMD_INLINE size_t Width() const
        {
            assert(_shape.size() == 4 && (_format == SimdTensorFormatNchw || _format == SimdTensorFormatNhwc));
            return _format == SimdTensorFormatNchw ? _shape[3] : _shape[2];
        }

        void DebugPrint(std::ostream & os, const String & name, size_t first = 5, size_t last = 2) const
        {
            os << name << " { ";
            for (size_t i = 0; i < _shape.size(); ++i)
                os << _shape[i] << " ";
            os << "} " << std::endl;

            if (_size == 0)
                return;

            size_t n = _shape.size();
            Test::Shape firsts(n), lasts(n), index(n, 0);
            Strings separators(n);
            for (ptrdiff_t i = n - 1; i >= 0; --i)
            {
                if (i == n - 1)
                {
                    firsts[i] = first;
                    lasts[i] = last;
                    separators[i] = "\t";
                }
                else
                {
                    firsts[i] = std::max<size_t>(firsts[i + 1] - 1, 1);
                    lasts[i] = std::max<size_t>(lasts[i + 1] - 1, 1);
                    separators[i] = separators[i + 1] + "\n";
                }
            }
            DebugPrint(os, firsts, lasts, separators, index, 0);
            if (n == 1 || n == 0)
                os << "\n";
        }

    private:

        void DebugPrint(std::ostream & os, const Test::Shape & firsts, const Test::Shape & lasts, const Strings & separators, Test::Shape index, size_t order) const
        {
            if (order == _shape.size())
            {
                std::cout << std::fixed << std::setprecision(4);
                os << *Data(index);
                return;
            }
            if (firsts[order] + lasts[order] < _shape[order])
            {
                size_t lo = firsts[order], hi = _shape[order] - lasts[order];
                for (index[order] = 0; index[order] < lo; ++index[order])
                {
                    DebugPrint(os, firsts, lasts, separators, index, order + 1);
                    os << separators[order];
                }
                os << "..." << separators[order];
                for (index[order] = hi; index[order] < _shape[order]; ++index[order])
                {
                    DebugPrint(os, firsts, lasts, separators, index, order + 1);
                    os << separators[order];
                }
            }
            else
            {
                for (index[order] = 0; index[order] < _shape[order]; ++index[order])
                {
                    DebugPrint(os, firsts, lasts, separators, index, order + 1);
                    os << separators[order];
                }
            }
        }

        SIMD_INLINE void Resize(const Type & value)
        {
            _size = Size(0, _shape.size());
            _data.resize(_size, value);
            SetDebugPtr();
        }

        SIMD_INLINE void Extend(const Type& value)
        {
            _size = Size(0, _shape.size());
            if (_size > _data.size())
                _data.resize(_size, value);
            SetDebugPtr();
        }

#if defined(_DEBUG) && defined(_MSC_VER)
        const Type * _ptr;

        SIMD_INLINE void SetDebugPtr()
        {
            _ptr = _data.data();
        }
#else
        SIMD_INLINE void SetDebugPtr()
        {
        }
#endif

        typedef std::vector<Type, Simd::Allocator<Type>> Vector;

        SimdTensorFormatType _format;
        Test::Shape _shape;
        size_t _size;
        Vector _data;
    };

    typedef Tensor<float> Tensor32f;
    typedef Tensor<uint8_t> Tensor8u;
    typedef Tensor<uint16_t> Tensor16u;

    //-------------------------------------------------------------------------------------------------

    inline Shape Shp()
    {
        return Shape();
    }

    inline Shape Shp(size_t axis0)
    {
        return Shape({ axis0 });
    }

    inline Shape Shp(size_t axis0, size_t axis1)
    {
        return Shape({ axis0, axis1 });
    }

    inline Shape Shp(size_t axis0, size_t axis1, size_t axis2)
    {
        return Shape({ axis0, axis1, axis2 });
    }

    inline Shape Shp(size_t axis0, size_t axis1, size_t axis2, size_t axis3)
    {
        return Shape({ axis0, axis1, axis2, axis3 });
    }

    inline Shape Shp(size_t axis0, size_t axis1, size_t axis2, size_t axis3, size_t axis4)
    {
        return Shape({ axis0, axis1, axis2, axis3, axis4 });
    }

    //-------------------------------------------------------------------------------------------------

    template<class T> inline void Copy(const Tensor<T> & src, Tensor<T> & dst)
    {
        assert(src.Size() == dst.Size());
        memcpy(dst.Data(), src.Data(), src.Size() * sizeof(T));
    }

    template<class T> inline void Fill(Tensor<T>& tensor, T value)
    {
        for (size_t i = 0; i < tensor.Size(); ++i)
            tensor.Data()[i] = value;
    }

    //-------------------------------------------------------------------------------------------------

    inline void Compare(const Tensor32f & a, const Tensor32f & b, float differenceMax, bool printError, int errorCountMax, DifferenceType differenceType, const String & description,
        Shape index, size_t order, int & errorCount, std::stringstream & message)
    {
        if (order == a.Count())
        {
            float _a = *a.Data(index);
            float _b = *b.Data(index);
            float absolute = ::fabs(_a - _b);
            float relative = ::fabs(_a - _b) / Simd::Max(::fabs(_a), ::fabs(_b));
            bool aNan = _a != _a;
            bool bNan = _b != _b;
            bool error = false;
            switch (differenceType)
            {
            case DifferenceAbsolute: error = absolute > differenceMax || aNan || bNan; break;
            case DifferenceRelative: error = relative > differenceMax; break;
            case DifferenceBoth: error = (absolute > differenceMax && relative > differenceMax) || aNan || bNan; break;
            case DifferenceAny: error = absolute > differenceMax || relative > differenceMax || aNan || bNan; break;
            case DifferenceLogical: error = aNan != bNan || (aNan == false && _a != _b); break;
            default:
                assert(0);
            }
            if (error)
            {
                errorCount++;
                if (printError)
                {
                    if (errorCount == 1)
                        message << std::endl << "Fail comparison: " << description << std::endl;
                    message << "Error at [";
                    for(size_t i = 0; i < index.size() - 1; ++i)
                        message << index[i] << ", ";
                    message << index[index.size() - 1] << "] : " << _a << " != " << _b << ";"
                        << " (absolute = " << absolute << ", relative = " << relative << ", threshold = " << differenceMax  << ")!" << std::endl;
                }
                if (errorCount > errorCountMax)
                {
                    if (printError)
                        message << "Stop comparison." << std::endl;
                }
            }
        }
        else
        {
            for (index[order] = 0; index[order] < a.Axis(order) && errorCount < errorCountMax; ++index[order])
                Compare(a, b, differenceMax, printError, errorCountMax, differenceType, description, index, order + 1, errorCount, message);
        }
    }

    inline bool Compare(const Tensor32f & a, const Tensor32f & b, float differenceMax, bool printError, int errorCountMax, DifferenceType differenceType, const String & description = "")
    {
        std::stringstream message;
        message << std::fixed << std::setprecision(6);
        int errorCount = 0;
        if (memcmp(a.Data(), b.Data(), a.Size() * sizeof(float)) == 0)
            return true;
        Index index(a.Count(), 0);
        Compare(a, b, differenceMax, printError, errorCountMax, differenceType, description, index, 0, errorCount, message);
        if (printError && errorCount > 0)
            TEST_LOG_SS(Error, message.str());
        return errorCount == 0;
    }

    template<class T> inline void Compare(const Tensor<T>& a, const Tensor<T>& b, int differenceMax, bool printError, int errorCountMax, const String& description,
        Shape index, size_t order, int& errorCount, std::stringstream& message)
    {
        if (order == a.Count())
        {
            int _a = *a.Data(index);
            int _b = *b.Data(index);
            int difference = Simd::Abs(_a - _b);
            bool error = difference > differenceMax;
            if (error)
            {
                errorCount++;
                if (printError)
                {
                    if (errorCount == 1)
                        message << std::endl << "Fail comparison: " << description << std::endl;
                    message << "Error at [";
                    for (size_t i = 0; i < index.size() - 1; ++i)
                        message << index[i] << ", ";
                    message << index[index.size() - 1] << "] : " << _a << " != " << _b << ";"
                        << " (difference = " << difference << ")!" << std::endl;
                }
                if (errorCount > errorCountMax)
                {
                    if (printError)
                        message << "Stop comparison." << std::endl;
                }
            }
        }
        else
        {
            for (index[order] = 0; index[order] < a.Axis(order) && errorCount < errorCountMax; ++index[order])
                Compare(a, b, differenceMax, printError, errorCountMax, description, index, order + 1, errorCount, message);
        }
    }

    template<class T> inline bool Compare(const Tensor<T>& a, const Tensor<T>& b, int differenceMax, bool printError, int errorCountMax, const String& description = "")
    {
        std::stringstream message;
        int errorCount = 0;
        Index index(a.Count(), 0);
        Compare(a, b, differenceMax, printError, errorCountMax, description, index, 0, errorCount, message);
        if (printError && errorCount > 0)
            TEST_LOG_SS(Error, message.str());
        return errorCount == 0;
    }

    //-------------------------------------------------------------------------------------------------

    inline void FillDebug(Tensor32f & dst, Shape index, size_t order)
    {
        if (order == dst.Count())
        {
            float value = 0.0f;
            for (int i = (int)index.size() - 1, n = 1; i >= 0; --i, n *= 10)
                value += float(index[i]%10*n);
            *dst.Data(index) = value;
        }
        else
        {
            for (index[order] = 0; index[order] < dst.Axis(order); ++index[order])
                FillDebug(dst, index, order + 1);
        }
    }

    inline void FillDebug(Tensor32f & dst)
    {
        Index index(dst.Count(), 0);
        FillDebug(dst, index, 0);
    }

    //-------------------------------------------------------------------------------------------------

    inline String ToString(SimdTensorFormatType format)
    {
        switch (format)
        {
        case SimdTensorFormatUnknown: return "Unknown";
        case SimdTensorFormatNchw: return "Nchw";
        case SimdTensorFormatNhwc: return "Nhwc";
        default: assert(0); return "Assert";
        }
    }

    inline String ToString(SimdTensorDataType data)
    {
        switch (data)
        {
        case SimdTensorDataUnknown: return "Unknown";
        case SimdTensorData32f: return "32f";
        case SimdTensorData32i: return "32i";
        case SimdTensorData8i: return "8i";
        case SimdTensorData8u: return "8u";
        case SimdTensorData64i: return "64i";
        case SimdTensorData64u: return "64u";
        case SimdTensorDataBool: return "Bool";
        case SimdTensorData16b: return "16b";
        case SimdTensorData16f: return "16f";
        default: assert(0); return "Assert";
        }
    }

    inline String ToChar(SimdTensorDataType data)
    {
        switch (data)
        {
        case SimdTensorDataUnknown: return "?";
        case SimdTensorData32f: return "f";
        case SimdTensorData32i: return "i";
        case SimdTensorData8i: return "u";
        case SimdTensorData8u: return "u";
        case SimdTensorData64i: return "l";
        case SimdTensorData64u: return "l";
        case SimdTensorDataBool: return "~";
        case SimdTensorData16b: return "b";
        case SimdTensorData16f: return "h";
        default: assert(0); return "Assert";
        }
    }

    //-------------------------------------------------------------------------------------------------

    inline Shape ToShape(size_t batch, size_t channels, size_t height, size_t width, SimdTensorFormatType format)
    {
        switch (format)
        {
        case SimdTensorFormatNchw: return Shp(batch, channels, height, width);
        case SimdTensorFormatNhwc: return Shp(batch, height, width, channels);
        default: assert(0); return Shape(0);
        }
    }

    inline Shape ToShape(size_t channels, size_t height, size_t width, SimdTensorFormatType format)
    {
        switch (format)
        {
        case SimdTensorFormatNchw: return Shp(channels, height, width);
        case SimdTensorFormatNhwc: return Shp(height, width, channels);
        default: assert(0); return Shape(0);
        }
    }

    inline Shape ToShape(size_t channels, size_t spatial, SimdTensorFormatType format)
    {
        switch (format)
        {
        case SimdTensorFormatNchw: return Shp(channels, spatial);
        case SimdTensorFormatNhwc: return Shp(spatial, channels);
        default: assert(0); return Shape(0);
        }
    }

    inline Shape ToShape(size_t value)
    {
        return Shape(1, value);
    }

    inline Shape ToShape(size_t channels, SimdTensorFormatType format)
    {
        return Shp(channels);
    }

    //-------------------------------------------------------------------------------------------------

    inline bool IsCompatible(const Shape& a, const Shape& b)
    {
        for (size_t i = 0, n = std::max(a.size(), b.size()), a0 = n - a.size(), b0 = n - b.size(); i < n; ++i)
        {
            size_t ai = i < a0 ? 1 : a[i - a0];
            size_t bi = i < b0 ? 1 : b[i - b0];
            if (!(ai == bi || ai == 1 || bi == 1))
                return false;
        }
        return true;
    }

   inline Shape OutputShape(const Shape& a, const Shape& b)
    {
        Shape d(std::max(a.size(), b.size()), 1);
        for (size_t i = 0, n = d.size(), a0 = n - a.size(), b0 = n - b.size(); i < n; ++i)
        {
            size_t ai = i < a0 ? 1 : a[i - a0];
            size_t bi = i < b0 ? 1 : b[i - b0];
            d[i] = std::max(ai, bi);
        }
        return d;
    }
}

#endif// __TestTensor_h__
