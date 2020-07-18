#include "qdp.h"

namespace QDP {

  extern llvm::IRBuilder<>  *builder;
  extern llvm::Module       *Mod;


  llvm::Function    *func_seed2float;
  llvm::Function    *func_seedMultiply;

  namespace JITSTATS {
    long lattice2dev  = 0;   // changing lattice data layout to device format
    long lattice2host = 0;   // changing lattice data layout to host format
    long jitted       = 0;   // functions not in DB, thus jit-built
  }


  void jit_stats_lattice2dev()  { ++JITSTATS::lattice2dev; }
  void jit_stats_lattice2host() { ++JITSTATS::lattice2host; }
  void jit_stats_jitted()       { ++JITSTATS::jitted; }

  long get_jit_stats_lattice2dev()  { return JITSTATS::lattice2dev; }
  long get_jit_stats_lattice2host() { return JITSTATS::lattice2host; }
  long get_jit_stats_jitted()       { return JITSTATS::jitted; }


  // seedMultiply
  //
  // We build a function that takes 2 seeds (4 ints each)
  // and returns 1 seed (as a literal aggregate)
  //
  void jit_build_seedMultiply() {
    assert(builder && "no builder");
    assert(Mod && "no module");

    std::vector< llvm::Type* > vecArgTypes;

    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );

    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );

    llvm::Type* ret_types[] = { builder->getInt32Ty(),
				builder->getInt32Ty(),
				builder->getInt32Ty(),
				builder->getInt32Ty() };
    
    llvm::StructType* ret_type = llvm::StructType::get(TheContext, 
						       llvm::ArrayRef< llvm::Type * >( ret_types , 4 ) );

    llvm::FunctionType *funcType = llvm::FunctionType::get( ret_type , 
							    llvm::ArrayRef<llvm::Type*>( vecArgTypes.data() , 
											 vecArgTypes.size() ) ,
							    false );
    llvm::Function* F = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, "seedMultiply", Mod);

    std::vector< llvm::Value* > args;
    unsigned Idx = 0;
    for (llvm::Function::arg_iterator AI = F->arg_begin(), AE = F->arg_end() ; AI != AE ; ++AI, ++Idx) {
      std::ostringstream oss;
      oss << "arg" << Idx;
      AI->setName( oss.str() );
      args.push_back(&*AI);
    }

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(TheContext, "entrypoint", F);
    builder->SetInsertPoint(entry);

    typedef RScalar<WordREG<int> >  T;
    PSeedREG<T> s1,s2;

    s1.elem(0).elem().setup( args[0] );
    s1.elem(1).elem().setup( args[1] );
    s1.elem(2).elem().setup( args[2] );
    s1.elem(3).elem().setup( args[3] );

    s2.elem(0).elem().setup( args[4] );
    s2.elem(1).elem().setup( args[5] );
    s2.elem(2).elem().setup( args[6] );
    s2.elem(3).elem().setup( args[7] );

    s1 = s1 * s2;

    llvm::Value* ret_val[] = { s1.elem(0).elem().get_val() ,
			       s1.elem(1).elem().get_val() ,
			       s1.elem(2).elem().get_val() ,
			       s1.elem(3).elem().get_val() };

    builder->CreateAggregateRet( ret_val , 4 );

    func_seedMultiply = F;
  }


  std::vector<llvm::Value *> llvm_seedMultiply( llvm::Value* a0 , llvm::Value* a1 , llvm::Value* a2 , llvm::Value* a3 , 
						llvm::Value* a4 , llvm::Value* a5 , llvm::Value* a6 , llvm::Value* a7 ) {
    assert(a0 && "llvm_seedToFloat a0");
    assert(a1 && "llvm_seedToFloat a1");
    assert(a2 && "llvm_seedToFloat a2");
    assert(a3 && "llvm_seedToFloat a3");
    assert(a4 && "llvm_seedToFloat a4");
    assert(a5 && "llvm_seedToFloat a5");
    assert(a6 && "llvm_seedToFloat a6");
    assert(a7 && "llvm_seedToFloat a7");

    assert(func_seedMultiply && "llvm_seedMultiply func_seedMultiply");

    llvm::Value* pack[] = { a0,a1,a2,a3,a4,a5,a6,a7 };

    llvm::Value* ret_val = builder->CreateCall( func_seedMultiply , llvm::ArrayRef< llvm::Value *>( pack ,  8 ) );

    std::vector<llvm::Value *> ret;
    ret.push_back( builder->CreateExtractValue( ret_val , 0 ) );
    ret.push_back( builder->CreateExtractValue( ret_val , 1 ) );
    ret.push_back( builder->CreateExtractValue( ret_val , 2 ) );
    ret.push_back( builder->CreateExtractValue( ret_val , 3 ) );

    return ret;
  }



  void jit_build_seedToFloat() {
    assert(builder && "no builder");
    assert(Mod && "no module");

    std::vector< llvm::Type* > vecArgTypes;
    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );
    vecArgTypes.push_back( builder->getInt32Ty() );

    llvm::FunctionType *funcType = llvm::FunctionType::get( builder->getFloatTy(), 
							    llvm::ArrayRef<llvm::Type*>( vecArgTypes.data() , 
											 vecArgTypes.size() ) ,
							    false );
    llvm::Function* F = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, "seedToFloat", Mod);

    std::vector< llvm::Value* > args;
    unsigned Idx = 0;
    for (llvm::Function::arg_iterator AI = F->arg_begin(), AE = F->arg_end() ; AI != AE ; ++AI, ++Idx) {
      std::ostringstream oss;
      oss << "arg" << Idx;
      AI->setName( oss.str() );
      args.push_back(&*AI);
    }

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(TheContext, "entrypoint", F);
    builder->SetInsertPoint(entry);

    typedef RScalar<WordREG<int> >  T;
    PSeedREG<T> s1;
    s1.elem(0).elem().setup( args[0] );
    s1.elem(1).elem().setup( args[1] );
    s1.elem(2).elem().setup( args[2] );
    s1.elem(3).elem().setup( args[3] );

     UnaryReturn<PSeedREG<T>, FnSeedToFloat>::Type_t  d; // QDP::PScalarREG<QDP::RScalarREG<QDP::WordREG<float> > >
    typedef  RealScalar<T>::Type_t  S;                                   // QDP::RScalarREG<QDP::WordREG<float> >

    S  twom11(1.0 / 2048.0);
    S  twom12(1.0 / 4096.0);
    S  fs1, fs2;

    //  recast_rep(fs1, s1.elem(0));
    fs1 = S(s1.elem(0));
    d.elem() = twom12 * S(s1.elem(0));

    //  recast_rep(fs1, s1.elem(1));
    fs1 = S(s1.elem(1));
    fs2 = fs1 + d.elem();
    d.elem() = twom12 * fs2;

    //  recast_rep(fs1, s1.elem(2));
    fs1 = S(s1.elem(2));
    fs2 = fs1 + d.elem();
    d.elem() = twom12 * fs2;

    //  recast_rep(fs1, s1.elem(3));
    fs1 = S(s1.elem(3));
    fs2 = fs1 + d.elem();
    d.elem() = twom11 * fs2;

    builder->CreateRet( d.elem().elem().get_val() );

    func_seed2float = F;
  }


  llvm::Value * llvm_seedToFloat( llvm::Value* a0 , llvm::Value* a1 , llvm::Value* a2 , llvm::Value* a3 ) {
    assert(a0 && "llvm_seedToFloat a0");
    assert(a1 && "llvm_seedToFloat a1");
    assert(a2 && "llvm_seedToFloat a2");
    assert(a3 && "llvm_seedToFloat a3");
    assert(func_seed2float && "llvm_seedToFloat func_seed2float");
    return builder->CreateCall( func_seed2float , {a0,a1,a2,a3} );
  }


  CUfunction jit_function_epilogue_get_cuf(const char * fname, const char* pretty )
  {
    llvm_exit();

    //llvm_module_dump();

    return llvm_get_cufunction( fname , pretty );
  }


  std::vector<ParamRef> jit_function_preamble_param()
  {
    llvm_start_new_function();

    ParamRef p_ordered      = llvm_add_param<bool>();
    ParamRef p_th_count     = llvm_add_param<int>();
    ParamRef p_start        = llvm_add_param<int>();
    ParamRef p_end          = llvm_add_param<int>();
    ParamRef p_member_array = llvm_add_param<bool*>();

    return { p_ordered , p_th_count , p_start , p_end , p_member_array };
  }


  llvm::Value *jit_function_preamble_get_idx( const std::vector<ParamRef>& vec )
  {
    llvm::Value * r_ordered      = llvm_derefParam( vec[0] );
    llvm::Value * r_th_count     = llvm_derefParam( vec[1] );
    llvm::Value * r_start        = llvm_derefParam( vec[2] );
                                   llvm_derefParam( vec[3]);     // r_end not used
    ParamRef      p_member_array = vec[4];

    llvm::Value * r_idx_phi0 = llvm_thread_idx();

    llvm::Value * r_idx_phi1;

    llvm_cond_exit( llvm_ge( r_idx_phi0 , r_th_count ) );

    llvm::BasicBlock * block_ordered = llvm_new_basic_block();
    llvm::BasicBlock * block_not_ordered = llvm_new_basic_block();
    llvm::BasicBlock * block_ordered_exit = llvm_new_basic_block();
    llvm::BasicBlock * cond_exit;
    llvm_cond_branch( r_ordered , block_ordered , block_not_ordered );
    {
      llvm_set_insert_point(block_not_ordered);
      llvm::Value* r_ismember     = llvm_array_type_indirection( p_member_array , r_idx_phi0 );
      llvm::Value* r_ismember_not = llvm_not( r_ismember );
      cond_exit = llvm_cond_exit( r_ismember_not ); 
      llvm_branch( block_ordered_exit );
    }
    {
      llvm_set_insert_point(block_ordered);
      r_idx_phi1 = llvm_add( r_idx_phi0 , r_start );
      llvm_branch( block_ordered_exit );
    }
    llvm_set_insert_point(block_ordered_exit);

    llvm::PHINode* r_idx = llvm_phi( r_idx_phi0->getType() , 2 );

    r_idx->addIncoming( r_idx_phi0 , cond_exit );
    r_idx->addIncoming( r_idx_phi1 , block_ordered );

    return r_idx;
  }


  JitForLoop::JitForLoop( int start , int end )
  {
    block_outer = llvm_get_insert_point();
    block_loop_cond = llvm_new_basic_block();
    block_loop_body = llvm_new_basic_block();
    block_loop_exit = llvm_new_basic_block();

    llvm::Value * i_start = llvm_create_value( start );
    
    llvm_branch( block_loop_cond );
    llvm_set_insert_point(block_loop_cond);
  
    r_i = llvm_phi( llvm_type<int>::value , 2 );

    r_i->addIncoming( i_start , block_outer );

    llvm_cond_branch( llvm_lt( r_i , llvm_create_value( end ) ) , block_loop_body , block_loop_exit );

    llvm_set_insert_point( block_loop_body );
  }
  llvm::Value * JitForLoop::index()
  {
    return r_i;
  }
  void JitForLoop::end()
  {
    llvm::Value * r_i_plus = llvm_add( r_i , llvm_create_value(1) );
    r_i->addIncoming( r_i_plus , llvm_get_insert_point() );
  
    llvm_branch( block_loop_cond );

    llvm_set_insert_point(block_loop_exit);
  }




  

  llvm::Value* llvm_epsilon_1st( int p1 , llvm::Value* j )
  {
    return llvm_rem( llvm_add( j , llvm_create_value( p1 ) ) , llvm_create_value( 3 ) );

  }
  
  llvm::Value* llvm_epsilon_2nd( int p2 , llvm::Value* i )
  {
    return llvm_rem( llvm_add( i , llvm_create_value( p2 ) ) , llvm_create_value( 3 ) );
  }


  void f1(int l,int r)
  {
    int i = (r + 1) % 3;
    int j = (l + 1) % 3;
    cout << "s1.elem(" << i << "," << j << ") * ";
  }






  
  
  
    // llvm::BasicBlock * block_start = llvm_new_basic_block();
  // llvm::BasicBlock * block_cont = llvm_new_basic_block();



  

} //namespace
