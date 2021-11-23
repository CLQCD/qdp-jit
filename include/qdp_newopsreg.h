#ifndef QDP_NEWOPTSREG_H
#define QDP_NEWOPTSREG_H


namespace QDP {

struct FnPeekColorMatrixREG
{
  FnPeekColorMatrixREG(ParamRef  _row, ParamRef  _col): row(_row), col(_col) {}
  
  template<class T>
  inline typename UnaryReturn<T, FnPeekColorMatrixREG>::Type_t
  operator()(const T &a) const
  {
    return peekColor(a,llvm_derefParam(row),llvm_derefParam(col));
  }
  
  llvm::Value* get_row_jit() const { return llvm_derefParam(row); }
  llvm::Value* get_col_jit() const { return llvm_derefParam(col); }

private:
  ParamRef row;
  ParamRef col;
};


struct FnPokeColorMatrixREG
{
  FnPokeColorMatrixREG(ParamRef  _row, ParamRef  _col): row(_row), col(_col) {}
  
  template<class T1, class T2>
  inline typename BinaryReturn<T1, T2, FnPokeColorMatrixREG>::Type_t
  operator()(const T1 &a, const T2 &b) const
  {
    pokeColor(const_cast<T1&>(a),b,llvm_derefParam(row),llvm_derefParam(col));
    return const_cast<T1&>(a);
  }

private:
  ParamRef  row;
  ParamRef col;
};




template<class A>
struct ForEach<UnaryNode<FnPeekColorMatrix, A>, ParamLeaf, TreeCombine>
{
  typedef typename ForEach<A, ParamLeaf, TreeCombine>::Type_t TypeA_t;
  typedef typename Combine1<TypeA_t, FnPeekColorMatrixREG, TreeCombine>::Type_t Type_t;
  inline static
  Type_t apply(const UnaryNode<FnPeekColorMatrix, A> &expr, const ParamLeaf &p, const TreeCombine &c)
  {
    ParamRef a = llvm_add_param<int>();
    ParamRef b = llvm_add_param<int>();
    return Combine1<TypeA_t, FnPeekColorMatrixREG, TreeCombine>::
      combine(ForEach<A, ParamLeaf, TreeCombine>::apply(expr.child(), p, c),
              FnPeekColorMatrixREG( a , b ) , c);
  }
};


template<class A>
struct ForEach<UnaryNode<FnPeekColorMatrix, A>, ParamLeafScalar, TreeCombine>
{
  typedef typename ForEach<A, ParamLeafScalar, TreeCombine>::Type_t TypeA_t;
  typedef typename Combine1<TypeA_t, FnPeekColorMatrixREG, TreeCombine>::Type_t Type_t;
  inline static
  Type_t apply(const UnaryNode<FnPeekColorMatrix, A> &expr, const ParamLeafScalar &p, const TreeCombine &c)
  {
    ParamRef a = llvm_add_param<int>();
    ParamRef b = llvm_add_param<int>();
    return Combine1<TypeA_t, FnPeekColorMatrixREG, TreeCombine>::
      combine(ForEach<A, ParamLeafScalar, TreeCombine>::apply(expr.child(), p, c),
              FnPeekColorMatrixREG( a , b ) , c);
  }
};

  
  

  template<>  
  struct AddOpParam< FnPokeColorMatrix, ParamLeaf> {
    typedef FnPokeColorMatrixREG Type_t;
    static FnPokeColorMatrixREG apply( const FnPokeColorMatrix& a, const ParamLeaf& p) {
      ParamRef pa = llvm_add_param<int>();
      ParamRef pb = llvm_add_param<int>();
      return FnPokeColorMatrixREG( pa , pb );
    }
  };

  

  template<>  
  struct AddOpAddress< FnPokeColorMatrix, AddressLeaf> {
    static void apply( const FnPokeColorMatrix& p, const AddressLeaf& a) {
      int row = p.getRow();
      int col = p.getCol();
      a.setLit( (int)row );
      a.setLit( (int)col );
    }
  };



template<class A>
struct ForEach<UnaryNode<FnPeekColorMatrix, A>, AddressLeaf, NullCombine>
{
    typedef typename ForEach< A , AddressLeaf, NullCombine>::Type_t TypeA_t;
    typedef TypeA_t Type_t;
    inline
    static Type_t apply(const UnaryNode<FnPeekColorMatrix, A>& expr, const AddressLeaf &a, const NullCombine &n)
    {
      int row = expr.operation().getRow();
      int col = expr.operation().getCol();
      a.setLit( (int)row );
      a.setLit( (int)col );
      return Type_t( ForEach<A, AddressLeaf, NullCombine>::apply( expr.child() , a , n ) );
    }
};


  //////////////////////////////

struct FnPeekSpinMatrixREG
{
  FnPeekSpinMatrixREG(ParamRef  _row, ParamRef  _col): row(_row), col(_col) {}

  template<class T>
  inline typename UnaryReturn<T, FnPeekSpinMatrixREG>::Type_t
  operator()(const T &a) const
  {
    return (peekSpin(a,llvm_derefParam(row),llvm_derefParam(col)));
  }

  llvm::Value* get_row_jit() const { return llvm_derefParam(row); }
  llvm::Value* get_col_jit() const { return llvm_derefParam(col); }
  
private:
  ParamRef  row;
  ParamRef  col;
};


struct FnPokeSpinMatrixREG
{
  FnPokeSpinMatrixREG(ParamRef  _row, ParamRef  _col): row(_row), col(_col) {}
  
  template<class T1, class T2>
  inline typename BinaryReturn<T1, T2, FnPokeSpinMatrixREG>::Type_t
  operator()(const T1 &a, const T2 &b) const
  {
    pokeSpin(const_cast<T1&>(a),b,llvm_derefParam(row),llvm_derefParam(col));
    return const_cast<T1&>(a);
  }


  template<class T1R, class T2R, int N1, int N2>
  inline typename BinaryReturn<PColorMatrixJIT<T1R,N1>, PColorMatrixREG<T2R,N2>, FnPokeSpinMatrixREG>::Type_t
  operator()(const PColorMatrixJIT<T1R,N1> &a, const PColorMatrixREG<T2R,N2> &b) const
  {
    const_cast<PColorMatrixJIT<T1R,N1>&>(a) = b;
    return const_cast<PColorMatrixJIT<T1R,N1>&>(a);
  }

  llvm::Value* jitRow() const { return llvm_derefParam(row); }
  llvm::Value* jitCol() const { return llvm_derefParam(col); }

private:
  ParamRef  row;
  ParamRef  col;
};





template<class A>
struct ForEach<UnaryNode<FnPeekSpinMatrix, A>, ParamLeaf, TreeCombine>
{
  typedef typename ForEach<A, ParamLeaf, TreeCombine>::Type_t TypeA_t;
  typedef typename Combine1<TypeA_t, FnPeekSpinMatrixREG, TreeCombine>::Type_t Type_t;
  inline static
  Type_t apply(const UnaryNode<FnPeekSpinMatrix, A> &expr, const ParamLeaf &p, const TreeCombine &c)
  {
    ParamRef a = llvm_add_param<int>();
    ParamRef b = llvm_add_param<int>();
    return Combine1<TypeA_t, FnPeekSpinMatrixREG, TreeCombine>::
      combine(ForEach<A, ParamLeaf, TreeCombine>::apply(expr.child(), p, c),
              FnPeekSpinMatrixREG( a , b ) , c);
  }
};


  template<class A>
  struct ForEach<UnaryNode<FnPeekSpinMatrix, A>, ParamLeafScalar, TreeCombine>
  {
    typedef typename ForEach<A, ParamLeafScalar, TreeCombine>::Type_t TypeA_t;
    typedef typename Combine1<TypeA_t, FnPeekSpinMatrixREG, TreeCombine>::Type_t Type_t;
    inline static
    Type_t apply(const UnaryNode<FnPeekSpinMatrix, A> &expr, const ParamLeafScalar &p, const TreeCombine &c)
    {
      ParamRef a = llvm_add_param<int>();
      ParamRef b = llvm_add_param<int>();
      return Combine1<TypeA_t, FnPeekSpinMatrixREG, TreeCombine>::
	combine(ForEach<A, ParamLeafScalar, TreeCombine>::apply(expr.child(), p, c),
		FnPeekSpinMatrixREG( a , b ) , c);
    }
  };

  
  
  template<>  
  struct AddOpParam< FnPokeSpinMatrix, ParamLeaf> {
    typedef FnPokeSpinMatrixREG Type_t;
    static FnPokeSpinMatrixREG apply( const FnPokeSpinMatrix& a, const ParamLeaf& p) {
      ParamRef pa = llvm_add_param<int>();
      ParamRef pb = llvm_add_param<int>();
      return FnPokeSpinMatrixREG( pa , pb );
    }
  };


  template<>  
  struct AddOpAddress< FnPokeSpinMatrix, AddressLeaf> {
    static void apply( const FnPokeSpinMatrix& p, const AddressLeaf& a) {
      int row = p.getRow();
      int col = p.getCol();
      a.setLit( (int)row );
      a.setLit( (int)col );
    }
  };



template<class A>
struct ForEach<UnaryNode<FnPeekSpinMatrix, A>, AddressLeaf, NullCombine>
{
    typedef typename ForEach< A , AddressLeaf, NullCombine>::Type_t TypeA_t;
    typedef TypeA_t Type_t;
    inline
    static Type_t apply(const UnaryNode<FnPeekSpinMatrix, A>& expr, const AddressLeaf &a, const NullCombine &n)
    {
      int row = expr.operation().getRow();
      int col = expr.operation().getCol();
      a.setLit( (int)row );
      a.setLit( (int)col );
      return Type_t( ForEach<A, AddressLeaf, NullCombine>::apply( expr.child() , a , n ) );
    }
};


  /////////////////////////////////////////////////
  ///
  ///
  /////////////////////////////////////////////////


struct FnPeekColorVectorREG
{
  FnPeekColorVectorREG(ParamRef  _row): row(_row) {}

  template<class T>
  inline typename UnaryReturn<T, FnPeekColorVectorREG>::Type_t
  operator()(const T &a) const
  {
    return (peekColor(a,llvm_derefParam(row)));
  }
  
  llvm::Value* get_row_jit() const { return llvm_derefParam(row); }

private:
  ParamRef  row;   // these are registers
};


struct FnPokeColorVectorREG
{
  FnPokeColorVectorREG(ParamRef  _row): row(_row) {}
  
  template<class T1, class T2>
  inline typename BinaryReturn<T1, T2, FnPokeColorVectorREG>::Type_t
  operator()(const T1 &a, const T2 &b) const
  {
    pokeColor(const_cast<T1&>(a),b,llvm_derefParam(row));
    return const_cast<T1&>(a);
  }

private:
  ParamRef  row;   // these are registers
};





template<class A>
struct ForEach<UnaryNode<FnPeekColorVector, A>, ParamLeaf, TreeCombine>
{
  typedef typename ForEach<A, ParamLeaf, TreeCombine>::Type_t TypeA_t;
  typedef typename Combine1<TypeA_t, FnPeekColorVectorREG, TreeCombine>::Type_t Type_t;
  inline static
  Type_t apply(const UnaryNode<FnPeekColorVector, A> &expr, const ParamLeaf &p, const TreeCombine &c)
  {
    ParamRef a = llvm_add_param<int>();
    return Combine1<TypeA_t, FnPeekColorVectorREG, TreeCombine>::
      combine(ForEach<A, ParamLeaf, TreeCombine>::apply(expr.child(), p, c),
              FnPeekColorVectorREG( a ) , c);
  }
};


  template<class A>
  struct ForEach<UnaryNode<FnPeekColorVector, A>, ParamLeafScalar, TreeCombine>
  {
    typedef typename ForEach<A, ParamLeafScalar, TreeCombine>::Type_t TypeA_t;
    typedef typename Combine1<TypeA_t, FnPeekColorVectorREG, TreeCombine>::Type_t Type_t;
    inline static
    Type_t apply(const UnaryNode<FnPeekColorVector, A> &expr, const ParamLeafScalar &p, const TreeCombine &c)
    {
      ParamRef a = llvm_add_param<int>();
      return Combine1<TypeA_t, FnPeekColorVectorREG, TreeCombine>::
	combine(ForEach<A, ParamLeafScalar, TreeCombine>::apply(expr.child(), p, c),
		FnPeekColorVectorREG( a ) , c);
    }
  };

  
  
  template<>  
  struct AddOpParam< FnPokeColorVector, ParamLeaf> {
    typedef FnPokeColorVectorREG Type_t;
    static FnPokeColorVectorREG apply( const FnPokeColorVector& a, const ParamLeaf& p) {
      return FnPokeColorVectorREG( llvm_add_param<int>() );
    }
  };


  template<>  
  struct AddOpAddress< FnPokeColorVector, AddressLeaf> {
    static void apply( const FnPokeColorVector& p, const AddressLeaf& a) {
      int row = p.getRow();
      a.setLit( (int)row );
    }
  };




template<class A>
struct ForEach<UnaryNode<FnPeekColorVector, A>, AddressLeaf, NullCombine>
{
    typedef typename ForEach< A , AddressLeaf, NullCombine>::Type_t TypeA_t;
    typedef TypeA_t Type_t;
    inline
    static Type_t apply(const UnaryNode<FnPeekColorVector, A>& expr, const AddressLeaf &a, const NullCombine &n)
    {
      int row = expr.operation().getRow();
      //std::cout << "set peek color vector row = " << row << "\n";
      a.setLit( (int)row );
      return Type_t( ForEach<A, AddressLeaf, NullCombine>::apply( expr.child() , a , n ) );
    }
};


  //////////////////////////////

struct FnPeekSpinVectorREG
{
  FnPeekSpinVectorREG(ParamRef  _row): row(_row) {}

  template<class T>
  inline typename UnaryReturn<T, FnPeekSpinVectorREG>::Type_t
  operator()(const T &a) const
  {
    return (peekSpin(a,llvm_derefParam(row)));
  }

  llvm::Value* get_row_jit() const { return llvm_derefParam(row); }

private:
  ParamRef  row;   // these are registers
};


struct FnPokeSpinVectorREG
{
  FnPokeSpinVectorREG(ParamRef  _row): row(_row) {}
  
  template<class T1, class T2>
  inline typename BinaryReturn<T1, T2, FnPokeSpinVectorREG>::Type_t
  operator()(const T1 &a, const T2 &b) const
  {
    pokeSpin(const_cast<T1&>(a),b,llvm_derefParam(row));
    return const_cast<T1&>(a);
  }

private:
  ParamRef  row;   // these are registers
};





template<class A>
struct ForEach<UnaryNode<FnPeekSpinVector, A>, ParamLeaf, TreeCombine>
{
  typedef typename ForEach<A, ParamLeaf, TreeCombine>::Type_t TypeA_t;
  typedef typename Combine1<TypeA_t, FnPeekSpinVectorREG, TreeCombine>::Type_t Type_t;
  inline static
  Type_t apply(const UnaryNode<FnPeekSpinVector, A> &expr, const ParamLeaf &p, const TreeCombine &c)
  {
    ParamRef a = llvm_add_param<int>();
    return Combine1<TypeA_t, FnPeekSpinVectorREG, TreeCombine>::
      combine(ForEach<A, ParamLeaf, TreeCombine>::apply(expr.child(), p, c),
              FnPeekSpinVectorREG( a ) , c);
  }
};

  template<class A>
  struct ForEach<UnaryNode<FnPeekSpinVector, A>, ParamLeafScalar, TreeCombine>
  {
    typedef typename ForEach<A, ParamLeafScalar, TreeCombine>::Type_t TypeA_t;
    typedef typename Combine1<TypeA_t, FnPeekSpinVectorREG, TreeCombine>::Type_t Type_t;
    inline static
    Type_t apply(const UnaryNode<FnPeekSpinVector, A> &expr, const ParamLeafScalar &p, const TreeCombine &c)
    {
      ParamRef a = llvm_add_param<int>();
      return Combine1<TypeA_t, FnPeekSpinVectorREG, TreeCombine>::
	combine(ForEach<A, ParamLeafScalar, TreeCombine>::apply(expr.child(), p, c),
		FnPeekSpinVectorREG( a ) , c);
    }
  };

  
  
  template<>  
  struct AddOpParam< FnPokeSpinVector, ParamLeaf> {
    typedef FnPokeSpinVectorREG Type_t;
    static FnPokeSpinVectorREG apply( const FnPokeSpinVector& a, const ParamLeaf& p) {
      return FnPokeSpinVectorREG( llvm_add_param<int>() );
    }
  };


  template<>  
  struct AddOpAddress< FnPokeSpinVector, AddressLeaf> {
    static void apply( const FnPokeSpinVector& p, const AddressLeaf& a) {
      int row = p.getRow();
      a.setLit( (int)row );
    }
  };



template<class A>
struct ForEach<UnaryNode<FnPeekSpinVector, A>, AddressLeaf, NullCombine>
{
    typedef typename ForEach< A , AddressLeaf, NullCombine>::Type_t TypeA_t;
    typedef TypeA_t Type_t;
    inline
    static Type_t apply(const UnaryNode<FnPeekSpinVector, A>& expr, const AddressLeaf &a, const NullCombine &n)
    {
      int row = expr.operation().getRow();
      //std::cout << "set peek spin vector row = " << row << "\n";
      a.setLit( (int)row );
      return Type_t( ForEach<A, AddressLeaf, NullCombine>::apply( expr.child() , a , n ) );
    }
};



template<>  
struct AddOpParam< FnPokeColorMatrix, ParamLeafScalar> {
  typedef FnPokeColorMatrixREG Type_t;
  static FnPokeColorMatrixREG apply( const FnPokeColorMatrix& a, const ParamLeafScalar& p) {
    ParamRef pa = llvm_add_param<int>();
    ParamRef pb = llvm_add_param<int>();
    return FnPokeColorMatrixREG( pa , pb );
  }
};
template<>  
struct AddOpParam< FnPokeSpinMatrix, ParamLeafScalar> {
  typedef FnPokeSpinMatrixREG Type_t;
  static FnPokeSpinMatrixREG apply( const FnPokeSpinMatrix& a, const ParamLeafScalar& p) {
    ParamRef pa = llvm_add_param<int>();
    ParamRef pb = llvm_add_param<int>();
    return FnPokeSpinMatrixREG( pa , pb );
  }
};
template<>  
struct AddOpParam< FnPokeColorVector, ParamLeafScalar> {
  typedef FnPokeColorVectorREG Type_t;
  static FnPokeColorVectorREG apply( const FnPokeColorVector& a, const ParamLeafScalar& p) {
    return FnPokeColorVectorREG( llvm_add_param<int>() );
  }
};
template<>  
struct AddOpParam< FnPokeSpinVector, ParamLeafScalar> {
  typedef FnPokeSpinVectorREG Type_t;
  static FnPokeSpinVectorREG apply( const FnPokeSpinVector& a, const ParamLeafScalar& p) {
    return FnPokeSpinVectorREG( llvm_add_param<int>() );
  }
};


}

#endif
