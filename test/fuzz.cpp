#include "../metric.h"
#include "../rgba_image.h"

#include <cassert>
#include <cmath>


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *const data,
                                      const size_t size)
{
#ifdef USE_PARAMETERS
    const auto parameter_size = sizeof(pdiff::PerceptualDiffParameters);
#else
    const size_t parameter_size = 0;
#endif

    const auto length = static_cast<size_t>(
        std::sqrt((size > parameter_size ?
                   size - parameter_size :
                   0) / 2));

    size_t data_offset = 0;

#ifdef USE_PARAMETERS
    pdiff::PerceptualDiffParameters parameters{};

    if (size >= parameter_size)
    {
        assert(2 * length * length + parameter_size <= size);

        std::memcpy(&parameters, data + data_offset, parameter_size);
        data_offset += parameter_size;
    }
#endif

    pdiff::RGBAImage image_a(length, length);
    pdiff::RGBAImage image_b(length, length);

    const auto image_size = length * length;

    if (image_size > 0)
    {
        memcpy(image_a.get_data(), data + data_offset, image_size);
        data_offset += image_size;
        assert(data_offset <= size);

        memcpy(image_b.get_data(), data + data_offset, image_size);
        data_offset += image_size;
        assert(data_offset <= size);
    }

    pdiff::yee_compare(image_a,
                       image_b
#ifdef USE_PARAMETERS 
                       ,
                       parameters
#endif
                       );

    return 0;
}
