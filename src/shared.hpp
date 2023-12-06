#ifndef SHARED_POINT
#define SHARED_POINT

#include <utility>

template <typename T>
class SharedPtr {
	private :
		T* _ptr;
		int* _cntPtr;
		void erase() {
			if(_ptr == nullptr || _cntPtr == nullptr) return;
			--(*_cntPtr);
			if(!(*_cntPtr)) {
				delete _ptr;
				_ptr = nullptr;
				delete _cntPtr;
				_cntPtr = nullptr;
			}
		}
	public :
		SharedPtr() : _ptr(nullptr), _cntPtr(nullptr) {};
		SharedPtr(T* ptr) {
			if(ptr == nullptr) {
				_ptr = nullptr;
				_cntPtr = nullptr;
				return;
			}
			_ptr = ptr;
			_cntPtr = new int(1);
		}
		SharedPtr(const SharedPtr& ptr) {
			if(ptr._ptr == nullptr) {
				_ptr = nullptr;
				_cntPtr = nullptr;
				return;
			}
			_ptr = ptr._ptr;
			_cntPtr = ptr._cntPtr;
			++(*_cntPtr);
		}
		~SharedPtr() {
			erase();
		}
		operator bool() const {
			return _ptr != nullptr;
		}
		SharedPtr& operator=(const SharedPtr& ptr) {
			reset(ptr);
			return *this;
		}
		T& operator*() const {
			return *_ptr;
		}
		T* operator->() const {
			return _ptr;
		}
		T* get() const {
			return _ptr;
		}
		void reset() {
			erase();
			_ptr = nullptr;
			_cntPtr = nullptr;
		}
		void reset(const SharedPtr& ptr) {
			if(_ptr == ptr._ptr) return;
			erase();
			_ptr = ptr._ptr;
			_cntPtr = ptr._cntPtr;
			if(_cntPtr != nullptr) ++(*_cntPtr);
		}
		int use_count() const {
			if(_cntPtr == nullptr) return 0;
			return *_cntPtr;
		}
};

template <typename T, typename... Ts>
SharedPtr<T> make_shared(Ts&&... args) {
	return SharedPtr<T>(new T(std::forward<Ts>(args)...));
}


#endif
