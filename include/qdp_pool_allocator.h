// -*- C++ -*-

#ifndef QDP_POOL_ALLOCATOR
#define QDP_POOL_ALLOCATOR

#include <string>
#include <list>
#include <iostream>
#include <algorithm>

using namespace std;


namespace QDP
{


  template<class Allocator>
  class QDPPoolAllocator {
  public:
    struct entry_t;

    static QDPPoolAllocator& Instance();

    typedef typename std::list< entry_t >              listEntry_t;
    typedef std::list< typename  listEntry_t::iterator> listEntryIter_t;

  public:

    void suspend();
    void resume();
    
    void registerMemory();
    void unregisterMemory();

    void   printListPool();
    void   printPoolInfo();
    size_t getPoolSize();

    bool allocate( void** ptr, size_t n_bytes );

    void free(const void *mem);
    void setPoolSize(size_t s);

    void enableMemset(unsigned val);

    QDPPoolAllocator();
    ~QDPPoolAllocator();

    bool allocateInternalBuffer();
    
  private:
    friend class QDPCache;


    QDPPoolAllocator(const QDPPoolAllocator&);                 // Prevent copy-construction
    QDPPoolAllocator& operator=(const QDPPoolAllocator&);

    void freeInternalBuffer();
    bool bufferAllocated;
    
    void *             poolPtr;
    void *             unaligned;
    size_t             poolSize;
    size_t             bytes_allocated;
    listEntry_t        listEntry;
    listEntryIter_t    listAllocOrder;
    typename listEntry_t::iterator iterNextNotAllocated;

    bool setMemory = false;
    unsigned setMemoryVal;

    bool findNextNotAllocated( typename listEntry_t::iterator & start , size_t & size );

  };


  template<class Allocator>
  void QDPPoolAllocator<Allocator>::registerMemory() {
      if (!bufferAllocated)
	allocateInternalBuffer();
      //QDP_info_primary("Pool allocator: Registering memory pool with NVIDIA driver (%lu bytes)",(unsigned long)bytes_allocated);
      HipHostRegister(unaligned,bytes_allocated);
    }


  template<class Allocator>
  void QDPPoolAllocator<Allocator>::unregisterMemory() {
      if (!bufferAllocated) {
	QDP_error_exit("pool unregisterMemory: not allocated");
      }
      QDP_info_primary("Pool allocator: Unregistering memory pool with NVIDIA driver");
      HipHostUnregister(unaligned);
    }


  template<class Allocator>
  QDPPoolAllocator<Allocator>& QDPPoolAllocator<Allocator>::Instance()
  {
    static QDPPoolAllocator singleton;
    return singleton;
  }


  template<class Allocator>
  struct QDPPoolAllocator<Allocator>::entry_t {
    void * ptr;
    size_t size;
    bool allocated;
  };


  template<class Allocator>
    QDPPoolAllocator<Allocator>::QDPPoolAllocator(): bufferAllocated(false) {
#ifdef GPU_DEBUG    
      QDP_debug("Pool allocator construct");
#endif      
      setPoolSize( 50*1024*1024 );
    }


  template<class Allocator>
  QDPPoolAllocator<Allocator>::~QDPPoolAllocator()
  { 
    freeInternalBuffer();
  }



  



  template<class T>
  struct SizeNotAllocated: public std::binary_function< typename T::entry_t, size_t , bool > {
    bool operator () ( const typename T::entry_t & ent , const size_t & size ) const {
      return ( 
	      (!ent.allocated) &&
	      (ent.size >= size) 
	       );
    }
  };


  template<class Allocator>
  void QDPPoolAllocator<Allocator>::freeInternalBuffer() {
    if (bufferAllocated) {
      //QDP_info_primary("pool allocator: Deallocating internal buffer");
      Allocator::free(unaligned);
      bufferAllocated=false;
    } else {
#ifdef GPU_DEBUG    
      QDP_debug("pool allocator: no internal buffer allocated");
#endif      
    }
  }

  template<class Allocator>
  void QDPPoolAllocator<Allocator>::suspend()
  {
    if (bufferAllocated) {
      Allocator::free( unaligned );
    }
  }

  template<class Allocator>
  void QDPPoolAllocator<Allocator>::resume()
  {
    if (!Allocator::allocate( (void**)&unaligned , bytes_allocated )) {
      QDPIO::cout << "upon resume, the pool allocater could not allocate " << bytes_allocated << "\n";
      QDP_error_exit("giving up");
    }
    poolPtr = (unsigned char *)( ( (unsigned long)unaligned + (QDP_ALIGNMENT_SIZE-1) ) & ~(QDP_ALIGNMENT_SIZE - 1));
  }


  template<class Allocator>
  bool QDPPoolAllocator<Allocator>::allocateInternalBuffer()
  {
    //QDP_info_primary("Pool allocator: allocate internal buffer (%lld)",(unsigned long long)poolSize);

    if (bufferAllocated) {
#ifdef GPU_DEBUG    
      QDP_debug("memory was allocated before, I will free it first..");
#endif      
      Allocator::free( unaligned );
      bufferAllocated = false;
#ifdef GPU_DEBUG      
      QDP_debug("listEntry size (should be 1) = %d" , listEntry.size());
#endif      
      if (listEntry.size() != 1)
	QDP_error_exit("pool allocator problem, listEntry not 1");
      while ( listEntry.size() )
	listEntry.pop_front();
    }

    if ( listEntry.size() > 0 )
      QDP_error_exit("Pool allocator: list of entries not zero");

    bytes_allocated = poolSize + 2 * QDP_ALIGNMENT_SIZE;
#ifdef GPU_DEBUG
    QDP_debug("Pool allocater: Allocating buffer %d bytes" , bytes_allocated );
#endif	
    if (!Allocator::allocate( (void**)&unaligned , bytes_allocated )) {
      QDPIO::cout << "Pool allocater could not allocate " << bytes_allocated << "\n";
      //QDP_error_exit("giving up");
      return false;
    }

    poolPtr = (unsigned char *)( ( (unsigned long)unaligned + (QDP_ALIGNMENT_SIZE-1) ) & ~(QDP_ALIGNMENT_SIZE - 1));
#ifdef GPU_DEBUG
    QDP_debug("pool allocator allocate internal buffer: unaligned ptr = %p  aligned ptr = %p" , (void*)unaligned , (void*)poolPtr);
#endif
    entry_t e;
    e.ptr = poolPtr;
    e.size = poolSize;
    e.allocated = false;
    listEntry.push_back(e);

    iterNextNotAllocated = listEntry.begin();

    if (setMemory)
      {
	assert(sizeof(unsigned) == 4);
	std::cout << "Initializing device memory with value = " << setMemoryVal << "\n";
	HipMemset( poolPtr , setMemoryVal , poolSize/sizeof(unsigned) );
      }
    
    bufferAllocated=true;
    return true;
  }
    

  template<class Allocator>
  void QDPPoolAllocator<Allocator>::printPoolInfo() {
    QDP_info("HIP memory allocated: start pointer = %p, size = %lu" , (void*)unaligned , (unsigned long)bytes_allocated );
  }


  template<class Allocator>
  void QDPPoolAllocator<Allocator>::printListPool() {
    QDP_info("Memory pool");
    int c=0;
    for ( typename listEntry_t::iterator p = listEntry.begin(); p != listEntry.end() ; p++ ) {

      typename listEntryIter_t::iterator q = listAllocOrder.begin();
      bool found=false;
      int d=0;
      while ((!found) && ( q != listAllocOrder.end() )) {
	if (*q == p)
	  found=true;
	else {
	  d++;
	  q++;
	}
      }

      if (iterNextNotAllocated==p)
	QDP_info("%d ptr=%p size=%d %d (cand) allocOrder=%d", c++ , p->ptr , p->size , p->allocated ,d);
      else
	QDP_info("%d ptr=%p size=%d %d allocOrder=%d", c++ , p->ptr , p->size , p->allocated ,d);
    }
  }


  template<class Allocator>
  bool QDPPoolAllocator<Allocator>::findNextNotAllocated( typename QDPPoolAllocator<Allocator>::listEntry_t::iterator & start , 
							  size_t & size ) {
    typename QDPPoolAllocator<Allocator>::listEntry_t::iterator save = start;
    start = std::find_if( start , listEntry.end(), std::bind2nd( SizeNotAllocated<QDPPoolAllocator<Allocator> >(), size ) );
    if ( start == listEntry.end()) {
      start = std::find_if( listEntry.begin() , save , std::bind2nd( SizeNotAllocated<QDPPoolAllocator<Allocator> >(), size ) );
      if ( start == save) {
	return false;
      }
    }
    return true;
  }


  template<class Allocator>
  bool QDPPoolAllocator<Allocator>::allocate( void ** ptr , size_t n_bytes ) {

    if (!bufferAllocated)
      {
	if (!allocateInternalBuffer())
	  return false;
      }

    //size_t alignment = QDP_ALIGNMENT_SIZE;
    size_t alignment = Allocator::ALIGNMENT_SIZE;

    size_t size = (n_bytes + (alignment) - 1) & ~((alignment) - 1);

#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep("Pool allocator: allocate=%lu (resized=%lu)", n_bytes , size );
#endif

    if (size > poolSize) {
      QDP_info("Pool allocator: trying to allocate %lu (poolsize=%lu) " , size , poolSize );
      return false;
    }

#ifdef GPU_DEBUG_DEEP
    if (size==0)
      QDP_error_exit("QDPPoolAllocator<Allocator>::allocate ( size == 0 )");
#endif

    typename QDPPoolAllocator::listEntry_t::iterator candidate = iterNextNotAllocated;
    if (candidate == listEntry.end() || candidate->allocated) {
      //QDP_info("Pool allocator (alignment=%u): no candidate, means pool full!",(unsigned)Allocator::ALIGNMENT_SIZE);
      //printListPool();
      return false;
    }

    do {
      if (candidate->size >= size) {
	if (candidate->size == size) {

	  candidate->allocated = true;

	  // we seek a spot of at least the mininmum size
	  findNextNotAllocated( iterNextNotAllocated , alignment );

	  listAllocOrder.push_front(candidate);
	  *ptr = candidate->ptr;

	  //std::cout << "pool alloc: " << *ptr << "\n";

	  return true;

	} else {

	  entry_t e;
	  e.ptr = candidate->ptr;
	  e.size = size;
	  e.allocated = true;
	
	  candidate->ptr = (void*)( (size_t)(candidate->ptr) + size );
	  candidate->size = candidate->size - size;

	  if (candidate->size==0)
	    QDP_error_exit("QDPPoolAllocator<Allocator>::allocate ( candidate->size == 0 ),%u",(unsigned)size);

	  iterNextNotAllocated = listEntry.insert( candidate , e );
	  listAllocOrder.push_front( iterNextNotAllocated );

	  iterNextNotAllocated++;

	  *ptr = e.ptr;

	  //std::cout << "pool alloc: " << *ptr << "\n";

	  return true;

	}
      }
    } while ( findNextNotAllocated( ++candidate , size ) );
#ifdef GPU_DEBUG
    QDP_debug("Pool allocator: out of memory");
#endif
    return false;
  }




  template<class Allocator>
  void QDPPoolAllocator<Allocator>::free(const void *mem) {

    typename listEntryIter_t::iterator q = listAllocOrder.begin();

    bool found=false;
    while ((!found) && ( q != listAllocOrder.end() )) {
      if ((*q)->ptr == mem) 
	found=true;
      else {
	q++;
      }
    }

    if (!found) {
      QDP_error_exit("pool allocator: free: address not found %p",mem);
    }

    typename listEntry_t::iterator p = *q;
    p->allocated = false;

    if ( p != listEntry.begin() ) {
      typename listEntry_t::iterator prev = p;
      prev--;
      if (!prev->allocated) {
	prev->size += p->size;
	listEntry.erase(p);
	p = prev;
      }
    }

    if ( p != --listEntry.end() ) {
      typename listEntry_t::iterator next = p;
      next++;
      if (!next->allocated) {
	p->size += next->size;
	listEntry.erase(next);
      }
    }

    iterNextNotAllocated = p;

    listAllocOrder.erase(q);

    return;
  }



  template<class Allocator>
  void QDPPoolAllocator<Allocator>::setPoolSize(size_t s) {
    //QDP_info_primary("Pool allocator: set pool size %lu bytes" , (unsigned long)s );
    poolSize = s;
  }

    template<class Allocator>
  void QDPPoolAllocator<Allocator>::enableMemset(unsigned val) {
      setMemory = true;
      setMemoryVal = val;
    }


  template<class Allocator>
  size_t QDPPoolAllocator<Allocator>::getPoolSize() {
    return poolSize;
  }




} // namespace QDP



#endif


