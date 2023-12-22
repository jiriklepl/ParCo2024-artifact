#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <cassert>
#include <memory>

#define CUCH(status)  do { cudaError_t err = status; if (err != cudaSuccess) std::cerr << __FILE__ ":" << __LINE__ << ": error: " << cudaGetErrorString(err) << "\n\t" #status << std::endl, exit(err); } while (false)

#if !defined(MINI_DATASET) && !defined(SMALL_DATASET) && !defined(MEDIUM_DATASET) && !defined(LARGE_DATASET) && !defined(EXTRALARGE_DATASET)
# error "Please define one of MINI_DATASET, SMALL_DATASET, MEDIUM_DATASET, LARGE_DATASET, EXTRALARGE_DATASET"
# define MEDIUM_DATASET
#endif

#if !defined(DATA_TYPE_IS_INT) && !defined(DATA_TYPE_IS_FLOAT) && !defined(DATA_TYPE_IS_DOUBLE)
# error "Please define one of DATA_TYPE_IS_INT, DATA_TYPE_IS_FLOAT, DATA_TYPE_IS_DOUBLE"
# define DATA_TYPE_IS_FLOAT
#endif

#if defined(DATA_TYPE_IS_FLOAT)
# define DATA_TYPE float
#elif defined(DATA_TYPE_IS_DOUBLE)
# define DATA_TYPE double
#endif

#include <noarr/structures_extended.hpp>
#include <noarr/structures/interop/bag.hpp>

inline void cudaInit() {
    cudaDeviceProp prop;
    CUCH(cudaGetDeviceProperties(&prop, 0));
    std::cerr << "Device name: " << prop.name << std::endl;

	CUCH(cudaSetDevice(0));
}

template<class Struct>
class managed_bag {
public:
    using value_type = noarr::scalar_t<Struct>;

    managed_bag(Struct layout) : _layout(layout), _data(nullptr), _data_device(nullptr) {
        _data = std::make_unique<value_type[]>((_layout | noarr::get_size()) / sizeof(value_type));
        CUCH(cudaMalloc(&_data_device, _layout | noarr::get_size()));
    }

    managed_bag(const managed_bag &other) = delete;
    managed_bag &operator=(const managed_bag &other) = delete;

    managed_bag(managed_bag &&other) noexcept : _layout(other._layout), _data(std::move(other._data)), _data_device(std::move(other._data_device)) {
        other._data = nullptr;
        other._data_device = nullptr;
    }

    managed_bag &operator=(managed_bag &&other) noexcept {
        _data = std::move(other._data);
        _data_device = std::move(other._data_device);
        other._data = nullptr;
        other._data_device = nullptr;

        _layout = other._layout;
        return *this;
    }

    void fetch_to_device() noexcept {
        assert(_data != nullptr);
        assert(_data_device != nullptr);
        CUCH(cudaMemcpy(_data_device, _data.get(), _layout | noarr::get_size(), cudaMemcpyDefault));
    }

    void fetch_to_host() noexcept {
        assert(_data != nullptr);
        assert(_data_device != nullptr);
        CUCH(cudaMemcpy(_data.get(), _data_device, _layout | noarr::get_size(), cudaMemcpyDefault));
    }

    ~managed_bag() noexcept {
        CUCH(cudaFree(_data_device));
        _data_device = nullptr;
    }

    auto get_host_ref() const noexcept {
        return noarr::make_bag(_layout, _data.get());
    }

    auto get_device_ref() const noexcept {
        return noarr::make_bag(_layout, _data_device);
    }

private:
    Struct _layout;
    std::unique_ptr<value_type[]> _data;
    value_type* _data_device;
};

#endif // DEFINES_HPP
