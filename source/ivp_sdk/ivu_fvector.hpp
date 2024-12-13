#ifndef _IVP_U_FVECTOR_INCLUDED_
#define _IVP_U_FVECTOR_INCLUDED_
#include "ivu_types.hpp"
#include "ivu_vector.hpp"

template<class T>
class IVP_U_FVector: public IVP_U_Vector_Base {
  private:
    void ensureCapacity() {
      if (nElems >= memSize) {
        this->incrementMem();
      }
    };
  
  protected:
    IVP_U_FVector(void **iElems, int size) {
      //IVP_ASSERT (iElems == (void **) (this + 1));
      elems = iElems;
      memSize = size;
      nElems = 0;
    }
  
  public:
    IVP_U_FVector(int size = 0) {
      memSize = size;
      nElems = 0;
      if (size) {
        elems = (void **) new char[size * sizeof(void *)];
      } else {	
        elems = NULL;
      }
    };
      
    void clear() {
      if ( elems != (void **) (this+1)) {
        if (elems) {
          delete[] (char *) elems;
          elems = NULL;
        }
        memSize = 0;
      }
      nElems = 0;
    };
      
    void removeAll() {
      nElems = 0;
    };
    
    ~IVP_U_FVector() {
      this->clear();
    };

    int len() const {
      return nElems;
    };
      
    int indexOf(T *elem) {
      int i = elem->getFVectorIndex(0);
      if ( i >= 0 && i < nElems && elementAt(i) == elem) return i;
      //IVP_ASSERT ( elem->getFVectorIndex(1) < 0 ||  elem->getFVectorIndex(1) >= nElems || elementAt(elem->getFVectorIndex(1)) == elem);
      return elem->getFVectorIndex(1);
    };
      
    int add(T *elem) {
      ensureCapacity();
      //IVP_ASSERT( indexOf(elem) == -1);
      elems[nElems] = (void *) elem;
      elem->setFVectorIndex(-1, nElems);
      return nElems++;
    };

    void swapElems(int index1, int index2) {
      //IVP_ASSERT((index1 >= 0) && (index1 < nElems));
      //IVP_ASSERT((index2 >= 0) && (index2 < nElems));
      T *a = (T*) elems[index1];
      T *b = (T*) elems[index2];
      elems[index1] = b;
      elems[index2] = a;
      a->setFVectorIndex(index1,index2);	
      b->setFVectorIndex(index2,index1);
      return;
    }    

    void removeAllowResort(T *elem) {
      int index = this->indexOf(elem);
      //IVP_ASSERT(index >= 0);
      nElems--;
      if ( nElems > index) {
        T *e = (T*) elems[nElems];
        elems[index] = e;
        e->setFVectorIndex(nElems, index);
      }
      elem->setFVectorIndex(index, -1);
    };
      

    T* elementAt(int index) const {
      //IVP_ASSERT(index >= 0 && index < nElems);
      return (T *) elems[index];
    };
};

#endif