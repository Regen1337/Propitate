#ifndef _IVP_U_VECTOR_INCLUDED_
#define _IVP_U_VECTOR_INCLUDED_

class IVP_U_Vector_Base {
  public:
    unsigned short memSize;
    unsigned short nElems;
    void **elems;
    void incrementMem();
};

template<class T>
class IVP_U_Vector: public IVP_U_Vector_Base {
  public:
    void ensureCapacity() {
      if (nElems >= memSize) {
        this->increment_mem();
      }
    };
  protected:
    IVP_U_Vector(void **iElems, int size) {
      //IVP_ASSERT (iElems == (void **) (this + 1));
      //IVP_ASSERT (size >= 0 && size <= 0xFFFFU);
      elems = iElems;
      memSize = (unsigned short) size;
      nElems = 0;
    }
    
  public:
    IVP_U_Vector(int size = 0) {
      //IVP_ASSERT (size >= 0 && size <= 0xFFFFU);
      memSize = (unsigned short) size;
      nElems = 0;
      if (size) {
        elems = (void **) new char[size * sizeof(void *)];
      } else {	
        elems = (void **) NULL;
      }
    };
    
    void clear() {
      if ( elems != (void **) (this + 1)){
        void *dummy = (void *) elems;
        if (elems) {
          delete[] (char *) elems;
          elems = NULL;
        }
        elems = 0;
        memSize = 0;
      }
      nElems = 0;
    };

    
    void removeAll() {
	    nElems = 0;
    };
    
    ~IVP_U_Vector() {
      this->clear();
    };

    int len() const {
      return nElems;
    };
    
    int indexOf(T *elem) {
      int i;
      for (i = nElems - 1; i >= 0; i--){
        if (elems[i] == elem) break;
      }
      return i;
    };
    
    int add(T *elem) {
      ensureCapacity();
      //IVP_ASSERT( indexOf(elem) == -1);
      elems[nElems] = (void *) elem;
      return nElems++;
    };

    int install(T *elem) {
      int old_index = indexOf(elem);
      if (old_index != -1) return old_index;
      ensureCapacity();
      elems[nElems] = (void *) elem;
      return nElems++;
    };

    void swapElems(int index1, int index2) {
      //IVP_ASSERT((index1 >= 0) && (index1 < nElems));
      //IVP_ASSERT((index2 >= 0) && (index2 < nElems));
      void *buffer = elems[index1];
      elems[index1] = elems[index2];
      elems[index2] = buffer;
      return;
    }     
    
    void insertAfter(int index, T *elem) {
      //IVP_ASSERT((index> = 0) && (index < nElems));
      index++;
      ensureCapacity();
      int j = nElems;
      while(j > index) {
        elems[j] = elems[j - 1];
        --j;
      }
      elems[index] = (void *) elem;
      nElems++;
    };

    
    void removeAt(int index) {
      //IVP_ASSERT((index >= 0) && (index < nElems));
      int j = index;
      while(j < nElems - 1) {
        elems[j] = elems[j + 1];
        j++;
      }
      nElems--;
    };

    void reverse() {
      for (int i = 0; i < (this->nElems / 2); i++) {
        this->swapElems(i, this->nElems - 1 - i);
      }
      return;
    }

    void removeAtAndAllowResort(int index) {
      //IVP_ASSERT((index >= 0) && (index < nElems));
      nElems--;
      elems[index] = elems[ nElems ];
    };

    void removeAllowResort(T *elem) {
      int index = this->indexOf(elem);
      //IVP_ASSERT(index >= 0);
      nElems--;
      elems[index] = elems[ nElems ];
    };
    
    void remove(T *elem) {
      int index = this->indexOf(elem);
      //IVP_ASSERT(index >= 0);
      nElems--;
      while (index < nElems) {
        elems[index] = (elems+1)[index];
        index++;
      }
    };

    T* elementAt(int index) const {
      //IVP_ASSERT(index >= 0 && index < nElems);
      return (T *) elems[index];
    };
};

template<class T>
class IVP_U_Vector_Enumerator {
  private:
    int index;
  
  public:
    inline IVP_U_Vector_Enumerator( IVP_U_Vector<T> *vec) {
      index = 0; //vec->nElems - 1;
    }

    T *getNextElement( IVP_U_Vector<T> *vec) {
      if (index >= vec->nElems) return NULL;
      //if (index < 0) return NULL;
      return vec->elementAt(index--);
    }
};

class IVP_Vector_of_Cores_2: public IVP_U_Vector<class IVP_Core> {
  private:
    IVP_Core *elemBuffer[2];
  
  public:
    IVP_Vector_of_Cores_2(): IVP_U_Vector<IVP_Core>( (void **) &elemBuffer[0], 2) {};
};

#endif