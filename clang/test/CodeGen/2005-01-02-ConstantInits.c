// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py UTC_ARGS: --function-signature --check-globals --global-value-regex "@.+"
// RUN: %clang_cc1 -triple=x86_64-unknown-linux %s -emit-llvm -o - | FileCheck %s

// This tests all kinds of hard cases with initializers and
// array subscripts.  This corresponds to PR487.

struct X { int a[2]; };

//.
// CHECK: @test.i23 = internal global i32 4, align 4
// CHECK: @i = global i32 4, align 4
// CHECK: @Arr = global [100 x i32] zeroinitializer, align 16
// CHECK: @foo2.X = internal global i32* bitcast (i8* getelementptr (i8, i8* bitcast ([100 x i32]* @Arr to i8*), i64 196) to i32*), align 8
// CHECK: @foo2.i23 = internal global i32 0, align 4
//.
// CHECK-LABEL: define {{[^@]+}}@test
// CHECK-SAME: () #[[ATTR0:[0-9]+]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = load i32, i32* @test.i23, align 4
// CHECK-NEXT:    ret i32 [[TMP0]]
//
int test() {
  static int i23 = (int) &(((struct X *)0)->a[1]);
  return i23;
}

int i = (int) &( ((struct X *)0) -> a[1]);

int Arr[100];

// CHECK-LABEL: define {{[^@]+}}@foo
// CHECK-SAME: (i32 noundef [[I:%.*]]) #[[ATTR0]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[I_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    store i32 [[I]], i32* [[I_ADDR]], align 4
// CHECK-NEXT:    [[CALL:%.*]] = call i32 (i32*, ...) bitcast (i32 (...)* @bar to i32 (i32*, ...)*)(i32* noundef getelementptr inbounds ([100 x i32], [100 x i32]* @Arr, i64 0, i64 49))
// CHECK-NEXT:    [[TMP0:%.*]] = load i32, i32* [[I_ADDR]], align 4
// CHECK-NEXT:    [[IDXPROM:%.*]] = sext i32 [[TMP0]] to i64
// CHECK-NEXT:    [[ARRAYIDX:%.*]] = getelementptr inbounds [100 x i32], [100 x i32]* @Arr, i64 0, i64 [[IDXPROM]]
// CHECK-NEXT:    [[CALL1:%.*]] = call i32 (i32*, ...) bitcast (i32 (...)* @bar to i32 (i32*, ...)*)(i32* noundef [[ARRAYIDX]])
// CHECK-NEXT:    [[ADD:%.*]] = add nsw i32 [[CALL]], [[CALL1]]
// CHECK-NEXT:    ret i32 [[ADD]]
//
int foo(int i) { return bar(&Arr[49])+bar(&Arr[i]); }
// CHECK-LABEL: define {{[^@]+}}@foo2
// CHECK-SAME: (i32 noundef [[I:%.*]]) #[[ATTR0]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[I_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[P:%.*]] = alloca i32*, align 8
// CHECK-NEXT:    store i32 [[I]], i32* [[I_ADDR]], align 4
// CHECK-NEXT:    store i32* getelementptr inbounds ([100 x i32], [100 x i32]* @Arr, i64 0, i64 0), i32** [[P]], align 8
// CHECK-NEXT:    [[TMP0:%.*]] = load i32*, i32** [[P]], align 8
// CHECK-NEXT:    [[INCDEC_PTR:%.*]] = getelementptr inbounds i32, i32* [[TMP0]], i32 1
// CHECK-NEXT:    store i32* [[INCDEC_PTR]], i32** [[P]], align 8
// CHECK-NEXT:    [[TMP1:%.*]] = load i32, i32* [[I_ADDR]], align 4
// CHECK-NEXT:    [[IDX_EXT:%.*]] = sext i32 [[TMP1]] to i64
// CHECK-NEXT:    [[ADD_PTR:%.*]] = getelementptr inbounds i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @Arr, i64 0, i64 0), i64 [[IDX_EXT]]
// CHECK-NEXT:    [[CALL:%.*]] = call i32 (i32*, ...) bitcast (i32 (...)* @bar to i32 (i32*, ...)*)(i32* noundef [[ADD_PTR]])
// CHECK-NEXT:    ret i32 [[CALL]]
//
int foo2(int i) {
  static const int *X = &Arr[49];
   static int i23 = (int) &( ((struct X *)0) -> a[0]);
  int *P = Arr;
  ++P;
  return bar(Arr+i);
}