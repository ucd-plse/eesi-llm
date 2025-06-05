#include "eesi_common.h"

#include "gtest/gtest.h"
#include "llvm/IR/GlobalVariable.h"

namespace error_specifications {

// Contains llvm::Value instances for testing.
class EESICommonTest : public ::testing::Test {
 protected:
  // Context
  llvm::LLVMContext llvm_context_;

  // Types
  llvm::PointerType *i8_ptr_type_ = llvm::Type::getInt8PtrTy(llvm_context_);
  llvm::IntegerType *i1_type_ = llvm::Type::getInt1Ty(llvm_context_);
  llvm::IntegerType *i32_type_ = llvm::Type::getInt32Ty(llvm_context_);

  // Booleans
  llvm::Constant *true_ = llvm::ConstantInt::getTrue(i1_type_);
  llvm::Constant *false_ = llvm::ConstantInt::getFalse(i1_type_);

  // Integers
  llvm::ConstantInt *zero_ = llvm::ConstantInt::get(i32_type_, 0, true);
  llvm::ConstantInt *one_ = llvm::ConstantInt::get(i32_type_, 1, true);
  llvm::ConstantInt *neg_one_ = llvm::ConstantInt::get(i32_type_, -1, true);

  // i8 null
  llvm::ConstantPointerNull *null_ =
      llvm::ConstantPointerNull::get(i8_ptr_type_);

  // String literal
  std::string string_value_ = "string";
  // The actual array holding the string data.
  llvm::Constant *string_initializer_ =
      llvm::ConstantDataArray::getString(llvm_context_, string_value_);
  // The program variable containing the string data.
  llvm::GlobalVariable *string_var_ = new llvm::GlobalVariable(
      string_initializer_->getType(), true,
      llvm::GlobalVariable::LinkageTypes::PrivateLinkage, string_initializer_);
  // getelementptr is used to decay the i8 array into a pointer whenever a
  // string literal is used as a char*.  Use this value when testing string
  // literals.
  llvm::Constant *string_use_ = llvm::ConstantExpr::getInBoundsGetElementPtr(
      nullptr, string_var_, llvm::ArrayRef<llvm::Constant *>({zero_, zero_}));

  void TearDown() override { delete string_var_; }
};

// Test that an llvm::ConstantInt is abstracted into the correct lattice
// element.
TEST_F(EESICommonTest, AbstractInteger) {
  ASSERT_EQ(AbstractInteger(*neg_one_),
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(AbstractInteger(*zero_),
            SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
  ASSERT_EQ(AbstractInteger(*one_),
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

// Test that the correct value is extracted from a boolean, which is represented
// by an llvm::ConstantInt with bitwidth 1.
TEST_F(EESICommonTest, ExtractBoolean) {
  const llvm::Optional<bool> maybe_true = ExtractBoolean(*true_);
  ASSERT_TRUE(maybe_true.hasValue());
  ASSERT_TRUE(*maybe_true);

  const llvm::Optional<bool> maybe_false = ExtractBoolean(*false_);
  ASSERT_TRUE(maybe_false.hasValue());
  ASSERT_FALSE(*maybe_false);
}

// Test that attempting to extract a boolean from an incompatible value will
// give no result.
TEST_F(EESICommonTest, ExtractBooleanInvalid) {
  const auto maybe_bool = ExtractBoolean(*string_use_);

  ASSERT_FALSE(maybe_bool.hasValue());
}

// Test that the correct value is extracted from an llvm::ConstantInt.
TEST_F(EESICommonTest, ExtractIntegerInt) {
  const llvm::Optional<std::int64_t> maybe_neg_one = ExtractInteger(*neg_one_);
  ASSERT_TRUE(maybe_neg_one.hasValue());
  ASSERT_EQ(*maybe_neg_one, -1);

  const llvm::Optional<std::int64_t> maybe_zero = ExtractInteger(*zero_);
  ASSERT_TRUE(maybe_zero.hasValue());
  ASSERT_EQ(*maybe_zero, 0);

  const llvm::Optional<std::int64_t> maybe_one = ExtractInteger(*one_);
  ASSERT_TRUE(maybe_one.hasValue());
  ASSERT_EQ(*maybe_one, 1);
}

// Test that zero is extracted from constant null.
TEST_F(EESICommonTest, ExtractIntegerNull) {
  const llvm::Optional<std::int64_t> maybe_zero = ExtractInteger(*null_);

  ASSERT_TRUE(maybe_zero.hasValue());
  ASSERT_EQ(*maybe_zero, 0);
}

// Test that attempting to extract an integer with an incompatible value will
// give no result.
TEST_F(EESICommonTest, ExtractIntegerInvalid) {
  const auto maybe_int = ExtractInteger(*string_use_);

  ASSERT_FALSE(maybe_int.hasValue());
}

// Test that the correct string is extracted from an llvm string literal use.
TEST_F(EESICommonTest, ExtractStringLiteral) {
  const auto maybe_string = ExtractStringLiteral(*string_use_);

  ASSERT_TRUE(maybe_string.hasValue());
  ASSERT_EQ(maybe_string->str(), string_value_);
}

// Test that attempting to extract a string literal from an incompatible value
// will give no result.
TEST_F(EESICommonTest, ExtractStringLiteralInvalid) {
  const auto maybe_string = ExtractStringLiteral(*one_);

  ASSERT_FALSE(maybe_string.hasValue());
}

}  // namespace error_specifications
