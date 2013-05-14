#ifndef QDP_LLVM
#define QDP_LLVM


#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/IR/DataLayout.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Support/raw_os_ostream.h"


namespace QDP {

  llvm::IRBuilder<> *builder;
  llvm::BasicBlock  *entry;
  llvm::Function    *mainFunc;
  llvm::Module      *Mod;

  llvm::Value * llvm_create_value( double v );
  llvm::Value * llvm_create_value( int v );
  llvm::Value * llvm_create_value( size_t v );
  llvm::Value * llvm_create_value( bool v );


  template<class T> struct llvm_type;

  template<> struct llvm_type<float> { static llvm::Type* value; };
  template<> struct llvm_type<double> { static llvm::Type* value; };
  template<> struct llvm_type<int> { static llvm::Type* value; };
  template<> struct llvm_type<bool> { static llvm::Type* value; };
  template<> struct llvm_type<float*> { static llvm::Type* value; };
  template<> struct llvm_type<double*> { static llvm::Type* value; };
  template<> struct llvm_type<int*> { static llvm::Type* value; };
  template<> struct llvm_type<bool*> { static llvm::Type* value; };



  void llvm_wrapper_init();
  llvm::PHINode * llvm_phi( llvm::Type* type, unsigned num = 0 );
  llvm::Type* promote( llvm::Type* t0 , llvm::Type* t1 );
  llvm::Value* llvm_cast( llvm::Type *dest_type , llvm::Value *src );

  std::tuple<llvm::Value*,llvm::Value*,llvm::Type*>
  llvm_normalize_values(llvm::Value* lhs , llvm::Value* rhs);

  llvm::Value* llvm_b_op( std::function< llvm::Value *(llvm::Value *, llvm::Value *) > func_float,
			  std::function< llvm::Value *(llvm::Value *, llvm::Value *) > func_int,
			  llvm::Value* lhs , llvm::Value* rhs );

  llvm::Value* llvm_u_op( std::function< llvm::Value *(llvm::Value *) > func_float,
			  std::function< llvm::Value *(llvm::Value *) > func_int,
			  llvm::Value* lhs );

  llvm::Value* llvm_xor( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_mul( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_add( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_sub( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_div( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_eq( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_ge( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_gt( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_le( llvm::Value* lhs , llvm::Value* rhs );
  llvm::Value* llvm_lt( llvm::Value* lhs , llvm::Value* rhs );

  //
  // Convenience function definitions
  //
  llvm::Value* llvm_not( llvm::Value* lhs );

  std::string param_next();


  template<class T> llvm::Argument *llvm_add_param();

  template<> llvm::Argument *llvm_add_param<bool>();
  template<> llvm::Argument *llvm_add_param<bool*>();
  template<> llvm::Argument *llvm_add_param<int>();
  template<> llvm::Argument *llvm_add_param<int*>();
  template<> llvm::Argument *llvm_add_param<float>();
  template<> llvm::Argument *llvm_add_param<float*>();
  template<> llvm::Argument *llvm_add_param<double>();
  template<> llvm::Argument *llvm_add_param<double*>();


  llvm::BasicBlock * llvm_new_basic_block();
  void llvm_cond_branch(llvm::Value * cond, llvm::BasicBlock * thenBB, llvm::BasicBlock * elseBB);
  void llvm_branch(llvm::BasicBlock * BB);
  void llvm_set_insert_point( llvm::BasicBlock * BB );
  llvm::BasicBlock * llvm_get_insert_point();
  void llvm_exit();
  void llvm_cond_exit( llvm::Value * cond );

  llvm::Value * llvm_create_value( double v );
  llvm::Value * llvm_create_value(int v );
  llvm::Value * llvm_create_value(size_t v);
  llvm::Value * llvm_create_value(bool v );


  llvm::Value * llvm_createGEP( llvm::Value * ptr , llvm::Value * idx );
  llvm::Value * llvm_load( llvm::Value * ptr );

  template<class T>
  llvm::Value* llvm_array_type_indirection( llvm::Value* idx );

  llvm::Value * llvm_special( const char * name );

  llvm::Value * llvm_call_special_tidx();
  llvm::Value * llvm_call_special_ntidx();
  llvm::Value * llvm_call_special_ctaidx();


  llvm::Value * llvm_thread_idx();

  void addKernelMetadata(llvm::Function *F);

  CUfunction llvm_get_cufunction(const char* fname);

} // namespace QDP


#endif
