// -*- C++ -*-
// $Id: scalar_specific.h,v 1.15 2002-11-13 19:04:42 edwards Exp $
//
// QDP data parallel interface
//
// Outer lattice routines specific to a scalar platform 

QDP_BEGIN_NAMESPACE(QDP);

// Use separate defs here. This will cause subroutine calls under g++

//-----------------------------------------------------------------------------
// Layout stuff specific to a scalar architecture
namespace Layout
{
};


//-----------------------------------------------------------------------------
//! OLattice Op Scalar(Expression(source)) under a subset
/*! 
 * OLattice Op Expression, where Op is some kind of binary operation 
 * involving the destination 
 */
template<class T, class T1, class Op, class RHS>
//inline
void evaluate(OLattice<T>& dest, const Op& op, const QDPExpr<RHS,OScalar<T1> >& rhs,
	      const Subset& s)
{
//  cerr << "In evaluate(olattice,oscalar)\n";

  const int *tab = s.SiteTable()->slice();
  for(int j=0; j < s.NumSiteTable(); ++j) 
  {
    int i = tab[j];
//    fprintf(stderr,"eval(olattice,oscalar): site %d\n",i);
//    op(dest.elem(i), forEach(rhs, ElemLeaf(), OpCombine()));
    op(dest.elem(i), forEach(rhs, EvalLeaf1(0), OpCombine()));
  }
}

//! OLattice Op Scalar(Expression(source)) under the ALL subset
/*! 
 * OLattice Op Expression, where Op is some kind of binary operation 
 * involving the destination 
 *
 * For now, always goes through the ALL subset
 * This helps with simplifying code development, but eventually
 * should be specific to the all subset
 */
template<class T, class T1, class Op, class RHS>
inline
void evaluate(OLattice<T>& dest, const Op& op, const QDPExpr<RHS,OScalar<T1> >& rhs)
{
  evaluate(dest, op, rhs, all);
}


//! OLattice Op OLattice(Expression(source)) under a subset
/*! 
 * OLattice Op Expression, where Op is some kind of binary operation 
 * involving the destination 
 */
template<class T, class T1, class Op, class RHS>
//inline
void evaluate(OLattice<T>& dest, const Op& op, const QDPExpr<RHS,OLattice<T1> >& rhs,
	      const Subset s)
{
//  cerr << "In evaluate(olattice,olattice)" << endl;

// NOTE: this code below is the first way I did the loop. The
// case when IndexRep is false is the optimal loop structure
// However, for simplicity and maintenance I will use the general
// form for all methods

//   if (! s.IndexRep())
//     for(int i=s.Start(); i <= s.End(); ++i) 
//     {
//       op(dest.elem(i), forEach(rhs, EvalLeaf1(0), OpCombine()));
//     }
//   else
//   {
//     const int *tab = s.SiteTable()->slice();
//     for(int j=0; j < s.NumSiteTable(); ++j) 
//     {
//       int i = tab[j];
//       op(dest.elem(i), forEach(rhs, EvalLeaf1(0), OpCombine()));
//     }
//   }
 
  // General form of loop structure
  const int *tab = s.SiteTable()->slice();
  for(int j=0; j < s.NumSiteTable(); ++j) 
  {
    int i = tab[j];
//    fprintf(stderr,"eval(olattice,olattice): site %d\n",i);
    op(dest.elem(i), forEach(rhs, EvalLeaf1(i), OpCombine()));
  }
}


//! OLattice Op OLattice(Expression(source))
template<class T, class T1, class Op, class RHS>
inline
void evaluate(OLattice<T>& dest, const Op& op, const QDPExpr<RHS,OLattice<T1> >& rhs)
{
  // Use the general loop form. However, could have an explicit loop
  // with no index lookup.
  evaluate(dest, op, rhs, all);
}



//-----------------------------------------------------------------------------
//! dest = (mask) ? s1 : dest
template<class T1, class T2> 
void copymask(OSubLattice<T2> d, const OLattice<T1>& mask, const OLattice<T2>& s1) 
{
  OLattice<T2>& dest = d.field();
  const Subset& s = d.subset();

  const int *tab = s.SiteTable()->slice();
  for(int j=0; j < s.NumSiteTable(); ++j) 
  {
    int i = tab[j];
    copymask(dest.elem(i), mask.elem(i), s1.elem(i));
  }
}

//! dest = (mask) ? s1 : dest
template<class T1, class T2> 
void copymask(OLattice<T2>& dest, const OLattice<T1>& mask, const OLattice<T2>& s1) 
{
  for(int i=0; i < Layout::vol(); ++i) 
    copymask(dest.elem(i), mask.elem(i), s1.elem(i));
}


//-----------------------------------------------------------------------------
// Auxilliary operations
//! coord[mu]  <- mu  : fill with lattice coord in mu direction
LatticeInteger latticeCoordinate(int mu);



//-----------------------------------------------------------------------------
// Random numbers
namespace RNG
{
  extern Seed ran_seed;
  extern Seed ran_mult;
  extern Seed ran_mult_n;
  extern LatticeSeed *lattice_ran_mult;
};


//! dest  = random  
/*! This implementation is correct for no inner grid */
template<class T>
void random(OScalar<T>& d)
{
  Seed seed = RNG::ran_seed;
  Seed skewed_seed = RNG::ran_seed * RNG::ran_mult;

  fill_random(d.elem(), seed, skewed_seed, RNG::ran_mult);

  RNG::ran_seed = seed;  // The seed from any site is the same as the new global seed
}


//! dest  = random    under a subset
template<class T>
void random(OSubLattice<T> dd)
{
  OLattice<T>& d = dd.field();
  const Subset& s = dd.subset();

  Seed seed;
  Seed skewed_seed;

  const int *tab = s.SiteTable()->slice();
  for(int j=0; j < s.NumSiteTable(); ++j) 
  {
    int i = tab[j];
    seed = RNG::ran_seed;
    skewed_seed.elem() = RNG::ran_seed.elem() * RNG::lattice_ran_mult->elem(i);
    fill_random(d.elem(i), seed, skewed_seed, RNG::ran_mult_n);
  }

  RNG::ran_seed = seed;  // The seed from any site is the same as the new global seed
}


//! dest  = random  
template<class T>
void random(OLattice<T>& d)
{
  random(d(all));
}


//! dest  = random   under a subset
template<class T>
void gaussian(OSubLattice<T> dd)
{
  OLattice<T>& d = dd.field();
  const Subset& s = dd.subset();

  OLattice<T>  r1, r2;

  random(r1(s));
  random(r2(s));

  const int *tab = s.SiteTable()->slice();
  for(int j=0; j < s.NumSiteTable(); ++j) 
  {
    int i = tab[j];
    fill_gaussian(d.elem(i), r1.elem(i), r2.elem(i));
  }
}


//! dest  = random  
template<class T>
void gaussian(OLattice<T>& d)
{
  gaussian(d(all));
}



//-----------------------------------------------------------------------------
// Broadcast operations
//! dest  = 0 
template<class T> 
inline
void zero_rep(OLattice<T>& dest, const Subset& s) 
{
  const int *tab = s.SiteTable()->slice();
  for(int j=0; j < s.NumSiteTable(); ++j) 
  {
    int i = tab[j];
    zero_rep(dest.elem(i));
  }
}


//! dest  = 0 
template<class T> 
void zero_rep(OSubLattice<T> dd) 
{
  OLattice<T>& d = dd.field();
  const Subset& s = dd.subset();
  
  zero_rep(d,s);
}


//! dest  = 0 
template<class T> 
void zero_rep(OLattice<T>& dest) 
{
  for(int i=0; i < Layout::vol(); ++i) 
    zero_rep(dest.elem(i));
}



//-----------------------------------------------
// Global sums
//! OScalar = sum(OScalar) under an explicit subset
/*!
 * Allow a global sum that sums over the lattice, but returns an object
 * of the same primitive type. E.g., contract only over lattice indices
 */
template<class RHS, class T>
typename UnaryReturn<OScalar<T>, FnSum>::Type_t
sum(const QDPExpr<RHS,OScalar<T> >& s1, const Subset& s)
{
  typename UnaryReturn<OScalar<T>, FnSum>::Type_t  d;

  evaluate(d,OpAssign(),s1);
  return d;
}


//! OScalar = sum(OScalar)
/*!
 * Allow a global sum that sums over the lattice, but returns an object
 * of the same primitive type. E.g., contract only over lattice indices
 */
template<class RHS, class T>
typename UnaryReturn<OScalar<T>, FnSum>::Type_t
sum(const QDPExpr<RHS,OScalar<T> >& s1)
{
  typename UnaryReturn<OScalar<T>, FnSum>::Type_t  d;

  evaluate(d,OpAssign(),s1);
  return d;
}



//! OScalar = sum(OLattice)  under an explicit subset
/*!
 * Allow a global sum that sums over the lattice, but returns an object
 * of the same primitive type. E.g., contract only over lattice indices
 */
template<class RHS, class T>
typename UnaryReturn<OLattice<T>, FnSum>::Type_t
sum(const QDPExpr<RHS,OLattice<T> >& s1, const Subset& s)
{
  typename UnaryReturn<OLattice<T>, FnSum>::Type_t  d;

  // Must initialize to zero since we do not know if the loop will be entered
  zero_rep(d.elem());

  const int *tab = s.SiteTable()->slice();
  for(int j=0; j < s.NumSiteTable(); ++j) 
  {
    int i = tab[j];
    d.elem() += forEach(s1, EvalLeaf1(i), OpCombine());   // SINGLE NODE VERSION FOR NOW
  }

  return d;
}


//! OScalar = sum(OLattice)
/*!
 * Allow a global sum that sums over the lattice, but returns an object
 * of the same primitive type. E.g., contract only over lattice indices
 */
template<class RHS, class T>
typename UnaryReturn<OLattice<T>, FnSum>::Type_t
sum(const QDPExpr<RHS,OLattice<T> >& s1)
{
  typename UnaryReturn<OLattice<T>, FnSum>::Type_t  d;

  // Loop always entered - could unroll
  zero_rep(d.elem());

  for(int i=0; i < Layout::vol(); ++i) 
    d.elem() += forEach(s1, EvalLeaf1(i), OpCombine());   // SINGLE NODE VERSION FOR NOW

  return d;
}


//-----------------------------------------------------------------------------
// Multiple global sums 
//! multi1d<OScalar> dest  = sumMulti(OScalar,Set) 
/*!
 * Compute the global sum on multiple subsets specified by Set 
 *
 * This implementation is specific to a purely olattice like
 * types. The scalar input value is replicated to all the
 * slices
 */
template<class RHS, class T>
typename UnaryReturn<OScalar<T>, FnSum>::Type_t
sumMulti(const QDPExpr<RHS,OScalar<T> >& s1, const Set& ss)
{
  typename UnaryReturn<OScalar<T>, FnSumMulti>::Type_t  dest(ss.NumSubsets());

  // lazy - evaluate repeatedly
  for(int i=0; i < ss.NumSubsets(); ++i)
  {
    evaluate(dest[i],OpAssign(),s1);
  }

  return dest;
}


//! multi1d<OScalar> dest  = sumMulti(OLattice,Set) 
/*!
 * Compute the global sum on multiple subsets specified by Set 
 *
 * This is a very simple implementation. There is no need for
 * anything fancier unless global sums are just so extraordinarily
 * slow. Otherwise, generalized sums happen so infrequently the slow
 * version is fine.
 */
template<class RHS, class T>
typename UnaryReturn<OLattice<T>, FnSumMulti>::Type_t
sumMulti(const QDPExpr<RHS,OLattice<T> >& s1, const Set& ss)
{
  typename UnaryReturn<OLattice<T>, FnSumMulti>::Type_t  dest(ss.NumSubsets());

  // Initialize result with zero
  for(int k=0; k < ss.NumSubsets(); ++k)
    zero_rep(dest[k]);

  // Loop over all sites and accumulate based on the coloring 
  const multi1d<int>& lat_color =  ss.LatticeColoring();

  for(int i=0; i < Layout::vol(); ++i) 
  {
    int j = lat_color[i];
    dest[j].elem() += forEach(s1, EvalLeaf1(i), OpCombine());   // SINGLE NODE VERSION FOR NOW
  }

  return dest;
}

//-----------------------------------------------------------------------------
// Peek and poke at individual sites. This is very architecture specific
// NOTE: these two routines assume there is no underlying inner grid

//! Extract site element
/*! @ingroup group1
  @param l  source to examine
  @param coord Nd lattice coordinates to examine
  @return single site object of the same primitive type
  @ingroup group1
  @relates QDPType */
template<class T1>
inline OScalar<T1>
peekSite(const OScalar<T1>& l, const multi1d<int>& coord)
{
  return l;
}


//! Extract site element
/*! @ingroup group1
  @param l  source to examine
  @param coord Nd lattice coordinates to examine
  @return single site object of the same primitive type
  @ingroup group1
  @relates QDPType */
template<class T1>
inline OScalar<T1>
peekSite(const OLattice<T1>& l, const multi1d<int>& coord)
{
  OScalar<T1> dest;

  dest.elem() = l.elem(Layout::linearSiteIndex(coord));
  return dest;
}


//! Insert site element
/*! @ingroup group1
  @param l  target to update
  @param r  source to insert
  @param coord Nd lattice coordinates where to insert
  @return object of the same primitive type but of promoted lattice type
  @ingroup group1
  @relates QDPType */
template<class T1>
inline OLattice<T1>&
pokeSite(OLattice<T1>& l, const OScalar<T1>& r, const multi1d<int>& coord)
{
  l.elem(Layout::linearSiteIndex(coord)) = r.elem();
  return l;
}


//-----------------------------------------------------------------------------
// Forward declaration
struct FnShift;

//! Nearest neighbor shift function class
class NearestNeighborMap
{
public:
  //! Constructor - does nothing really
  NearestNeighborMap() {}

  //! Destructor
  ~NearestNeighborMap() {}

  //! Actual constructor
  void make();

  //! Function call operator for a shift
  /*! 
   * shift(source,isign,dir)
   * isign = parity of direction (+1 or -1)
   * dir   = direction ([0,...,Nd-1])
   *
   * Implements:  dest(x) = s1(x+isign*dir)
   * There are cpp macros called  FORWARD and BACKWARD that are +1,-1 resp.
   * that are often used as arguments
   *
   * Shifts on a OLattice are non-trivial.
   * Notice, there may be an ILattice underneath which requires shift args.
   * This routine is very architecture dependent.
   */
  template<class T1,class C1>
  inline typename MakeReturn<UnaryNode<FnShift,
    typename CreateLeaf<QDPType<T1,C1> >::Leaf_t>, C1>::Expression_t
  operator()(const QDPType<T1,C1> & l, int isign, int dir)
    {
      //  fprintf(stderr,"shift(QDPType,%d,%d)\n",isign,dir);

      typedef UnaryNode<FnShift,
	typename CreateLeaf<QDPType<T1,C1> >::Leaf_t> Tree_t;
      return MakeReturn<Tree_t,C1>::make(Tree_t(FnShift(isign,dir),
	CreateLeaf<QDPType<T1,C1> >::make(l)));
    }


  template<class T1,class C1>
  inline typename MakeReturn<UnaryNode<FnShift,
    typename CreateLeaf<QDPExpr<T1,C1> >::Leaf_t>, C1>::Expression_t
  operator()(const QDPExpr<T1,C1> & l, int isign, int dir)
    {
      //  fprintf(stderr,"shift(QDPExpr,%d,%d)\n",isign,dir);

      typedef UnaryNode<FnShift,
	typename CreateLeaf<QDPExpr<T1,C1> >::Leaf_t> Tree_t;
      return MakeReturn<Tree_t,C1>::make(Tree_t(FnShift(isign,dir),
	CreateLeaf<QDPExpr<T1,C1> >::make(l)));
    }


public:
  //! Accessor to offsets
  const multi3d<int>& Offsets() const {return soffsets;}

private:
  //! Hide copy constructor
  NearestNeighborMap(const NearestNeighborMap&) {}

private:
  //! Offset table used for communications. 
  /*! 
   * The direction is in the sense of the Map or Shift functions from QDP.
   * soffsets(direction,isign,position) 
   */ 
  multi3d<int> soffsets;
};



// This is the PETE version of a shift, namely return an expression
//
// This class looks like those in OperatorTags, but has a specific constructor
// for a given direction
// This mechanism needs to be more general - this implementation is a prototype.
//
// NOTE: the use of "all" is not desired. The offsets is not suppose to
// be subset dependent, but is general to the class. E.g., all shifts should be
// static classes
struct FnShift
{
  PETE_EMPTY_CONSTRUCTORS(FnShift)

  const int *soff;
  FnShift(int isign, int dir): soff(shift.Offsets().slice(dir,(isign+1)>>1))
    {
//      fprintf(stderr,"FnShift(%d,%d): soff=0x%x\n",isign,dir,soff);
    }
  
  template<class T>
  inline typename UnaryReturn<T, FnShift>::Type_t
  operator()(const T &a) const
  {
    return (a);
  }
};


// Specialization of ForEach deals with shifts. 
// This mechanism needs to be more general - this implementation is a prototype.
template<class A, class CTag>
struct ForEach<UnaryNode<FnShift, A>, EvalLeaf1, CTag>
{
  typedef typename ForEach<A, EvalLeaf1, CTag>::Type_t TypeA_t;
  typedef typename Combine1<TypeA_t, FnShift, CTag>::Type_t Type_t;
  inline static
  Type_t apply(const UnaryNode<FnShift, A> &expr, const EvalLeaf1 &f, 
    const CTag &c) 
  {
    EvalLeaf1 ff(expr.operation().soff[f.val1()]);
//    fprintf(stderr,"ForEach<Unary<FnShift>>: site = %d, new = %d\n",f.val1(),ff.val1());

    return Combine1<TypeA_t, FnShift, CTag>::
      combine(ForEach<A, EvalLeaf1, CTag>::apply(expr.child(), ff, c),
              expr.operation(), c);
  }
};


//-----------------------------------------------------------------------------
// Input and output of various flavors that are architecture specific

//! Namelist output
template<class T>  
NmlWriter& operator<<(NmlWriter& nml, const OLattice<T>& d)
{
  nml.get() << "   [OUTER]" << endl;
  for(int site=0; site < Layout::vol(); ++site) 
  {
    int i = Layout::linearSiteIndex(site);
    nml.get() << "   Site =  " << site << "   = ";
    nml << d.elem(i);
    nml.get() << "," << endl;
  }

//  int site = Layout::vol()-1;
//  int i = Layout::linearSiteIndex(site);
//  nml << "   Site =  " << site << "   = " << d.elem(i) << ",\n";

  return nml;
}


//! Binary output
template<class T>  
BinaryWriter& write(BinaryWriter& bin, const OLattice<T>& d)
{
  for(int site=0; site < Layout::vol(); ++site) 
  {
    int i = Layout::linearSiteIndex(site);
    bfwrite((void *)&(d.elem(i)), sizeof(typename WordType<T>::Type_t), 
	    sizeof(T) / sizeof(typename WordType<T>::Type_t), bin.get());
  }

  return bin;
}

//! Generic read a binary element
template<class T>
BinaryReader& read(BinaryReader& bin, T& d)
{
  if (bfread((void *)&d, sizeof(T), 1, bin.get()) != 1)
    QDP_error_exit("BinaryReader: failed to read");

  return bin;
}

//! Binary input
/*! Assumes no inner grid */
template<class T>
BinaryReader& read(BinaryReader& bin, OScalar<T>& d)
{
  bfread((void *)&(d.elem()), sizeof(typename WordType<T>::Type_t), 
	 sizeof(T) / sizeof(typename WordType<T>::Type_t), bin.get()); 

  return bin;
}

//! Binary input
/*! Assumes no inner grid */
template<class T>  
BinaryReader& read(BinaryReader& bin, OLattice<T>& d)
{
  for(int site=0; site < Layout::vol(); ++site) 
  {
    int i = Layout::linearSiteIndex(site);
    bfread((void *)&(d.elem(i)), sizeof(typename WordType<T>::Type_t), 
	   sizeof(T) / sizeof(typename WordType<T>::Type_t), bin.get());
  }

  return bin;
}

QDP_END_NAMESPACE();
