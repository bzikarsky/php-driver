/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "php_driver.h"
#include "php_driver_types.h"
#include "util/hash.h"
#include "util/math.h"
#include "util/types.h"

#if !defined(HAVE_STDINT_H) && !defined(_MSC_STDINT_H_)
#  define INT64_MAX 9223372036854775807LL
#  define INT64_MIN (-INT64_MAX-1)
#endif

zend_class_entry *php_driver_bigint_ce = NULL;

static int
to_double(zval *result, php_driver_numeric *bigint)
{
  ZVAL_DOUBLE(result, (double) bigint->data.bigint.value);
  return SUCCESS;
}

static int
to_long(zval *result, php_driver_numeric *bigint)
{
  if (bigint->data.bigint.value < (cass_int64_t) ZEND_LONG_MIN) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too small");
    return FAILURE;
  }

  if (bigint->data.bigint.value > (cass_int64_t) ZEND_LONG_MAX) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value is too big");
    return FAILURE;
  }

  ZVAL_LONG(result, (zend_long) bigint->data.bigint.value);
  return SUCCESS;
}

static int
to_string(zval *result, php_driver_numeric *bigint)
{
  char *string;
  spprintf(&string, 0, LL_FORMAT, bigint->data.bigint.value);
  ZVAL_STRING(result, string);
  efree(string);
  return SUCCESS;
}

void
php_driver_bigint_init(INTERNAL_FUNCTION_PARAMETERS)
{
  php_driver_numeric *self;
  zval *value;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &value) == FAILURE) {
    return;
  }

  if (getThis() && instanceof_function(Z_OBJCE_P(getThis()), php_driver_bigint_ce)) {
    self = PHP_DRIVER_GET_NUMERIC(getThis());
  } else {
    object_init_ex(return_value, php_driver_bigint_ce);
    self = PHP_DRIVER_GET_NUMERIC(return_value);
  }

  if (Z_TYPE_P(value) == IS_LONG) {
    self->data.bigint.value = (cass_int64_t) Z_LVAL_P(value);
  } else if (Z_TYPE_P(value) == IS_DOUBLE) {
    double double_value = Z_DVAL_P(value);

    if (double_value > INT64_MAX || double_value < INT64_MIN) {
      zend_throw_exception_ex(php_driver_range_exception_ce, 0,
        "value must be between " LL_FORMAT " and " LL_FORMAT ", %g given",
        INT64_MIN, INT64_MAX, double_value);
      return;
    }

    self->data.bigint.value = (cass_int64_t) Z_DVAL_P(value);
  } else if (Z_TYPE_P(value) == IS_STRING) {
    if (!php_driver_parse_bigint(Z_STRVAL_P(value), Z_STRLEN_P(value),
                                    &self->data.bigint.value)) {
      return;
    }
  } else if (Z_TYPE_P(value) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(value), php_driver_bigint_ce)) {
    php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(value);
    self->data.bigint.value = bigint->data.bigint.value;
  } else {
    INVALID_ARGUMENT(value, "a long, a double, a numeric string or a " \
                            PHP_DRIVER_NAMESPACE "\\Bigint");
  }
}

/* {{{ Bigint::__construct(string) */
PHP_METHOD(Bigint, __construct)
{
  php_driver_bigint_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Bigint::__toString() */
PHP_METHOD(Bigint, __toString)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());

  to_string(return_value, self);
}
/* }}} */

/* {{{ Bigint::type() */
PHP_METHOD(Bigint, type)
{
  zval type = php_driver_type_scalar(CASS_VALUE_TYPE_BIGINT);
  RETURN_ZVAL(&(type), 1, 1);
}
/* }}} */

/* {{{ Bigint::value() */
PHP_METHOD(Bigint, value)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());

  to_string(return_value, self);
}
/* }}} */

/* {{{ Bigint::add() */
PHP_METHOD(Bigint, add)
{
  zval *num;
  php_driver_numeric *self;
  php_driver_numeric *bigint;
  php_driver_numeric *result;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    self = PHP_DRIVER_GET_NUMERIC(getThis());
    bigint = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_bigint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    result->data.bigint.value = self->data.bigint.value + bigint->data.bigint.value;
  } else {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }
}
/* }}} */

/* {{{ Bigint::sub() */
PHP_METHOD(Bigint, sub)
{
  zval *num;
  php_driver_numeric *result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());
    php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_bigint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    result->data.bigint.value = self->data.bigint.value - bigint->data.bigint.value;
  } else {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }
}
/* }}} */

/* {{{ Bigint::mul() */
PHP_METHOD(Bigint, mul)
{
  zval *num;
  php_driver_numeric *result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());
    php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_bigint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    result->data.bigint.value = self->data.bigint.value * bigint->data.bigint.value;
  } else {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }
}
/* }}} */

/* {{{ Bigint::div() */
PHP_METHOD(Bigint, div)
{
  zval *num;
  php_driver_numeric *result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());
    php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_bigint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    if (bigint->data.bigint.value == 0) {
      zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot divide by zero");
      return;
    }

    result->data.bigint.value = self->data.bigint.value / bigint->data.bigint.value;
  } else {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }
}
/* }}} */

/* {{{ Bigint::mod() */
PHP_METHOD(Bigint, mod)
{
  zval *num;
  php_driver_numeric *result = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &num) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(num) == IS_OBJECT &&
      instanceof_function(Z_OBJCE_P(num), php_driver_bigint_ce)) {
    php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());
    php_driver_numeric *bigint = PHP_DRIVER_GET_NUMERIC(num);

    object_init_ex(return_value, php_driver_bigint_ce);
    result = PHP_DRIVER_GET_NUMERIC(return_value);

    if (bigint->data.bigint.value == 0) {
      zend_throw_exception_ex(php_driver_divide_by_zero_exception_ce, 0, "Cannot modulo by zero");
      return;
    }

    result->data.bigint.value = self->data.bigint.value % bigint->data.bigint.value;
  } else {
    INVALID_ARGUMENT(num, "a " PHP_DRIVER_NAMESPACE "\\Bigint");
  }
}
/* }}} */

/* {{{ Bigint::abs() */
PHP_METHOD(Bigint, abs)
{
  php_driver_numeric *result = NULL;
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());

  if (self->data.bigint.value == INT64_MIN) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0, "Value doesn't exist");
    return;
  }

  object_init_ex(return_value, php_driver_bigint_ce);
  result = PHP_DRIVER_GET_NUMERIC(return_value);
  result->data.bigint.value = self->data.bigint.value < 0 ? -self->data.bigint.value : self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::neg() */
PHP_METHOD(Bigint, neg)
{
  php_driver_numeric *result = NULL;
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());

  object_init_ex(return_value, php_driver_bigint_ce);
  result = PHP_DRIVER_GET_NUMERIC(return_value);
  result->data.bigint.value = -self->data.bigint.value;
}
/* }}} */

/* {{{ Bigint::sqrt() */
PHP_METHOD(Bigint, sqrt)
{
  php_driver_numeric *result = NULL;
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());

  if (self->data.bigint.value < 0) {
    zend_throw_exception_ex(php_driver_range_exception_ce, 0,
                            "Cannot take a square root of a negative number");
  }

  object_init_ex(return_value, php_driver_bigint_ce);
  result = PHP_DRIVER_GET_NUMERIC(return_value);
  result->data.bigint.value = (cass_int64_t) sqrt((long double) self->data.bigint.value);
}
/* }}} */

/* {{{ Bigint::toInt() */
PHP_METHOD(Bigint, toInt)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());

  to_long(return_value, self);
}
/* }}} */

/* {{{ Bigint::toDouble() */
PHP_METHOD(Bigint, toDouble)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(getThis());

  to_double(return_value, self);
}
/* }}} */

/* {{{ Bigint::min() */
PHP_METHOD(Bigint, min)
{
  php_driver_numeric *bigint = NULL;
  object_init_ex(return_value, php_driver_bigint_ce);
  bigint = PHP_DRIVER_GET_NUMERIC(return_value);
  bigint->data.bigint.value = INT64_MIN;
}
/* }}} */

/* {{{ Bigint::max() */
PHP_METHOD(Bigint, max)
{
  php_driver_numeric *bigint = NULL;
  object_init_ex(return_value, php_driver_bigint_ce);
  bigint = PHP_DRIVER_GET_NUMERIC(return_value);
  bigint->data.bigint.value = INT64_MAX;
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo__construct, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_num, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_bigint_methods[] = {
  PHP_ME(Bigint, __construct, arginfo__construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, __toString, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, type, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, value, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, add, arginfo_num, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, sub, arginfo_num, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, mul, arginfo_num, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, div, arginfo_num, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, mod, arginfo_num, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, abs, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, neg, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, sqrt, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, toInt, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, toDouble, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(Bigint, min, arginfo_none, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_ME(Bigint, max, arginfo_none, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
  PHP_FE_END
};

static php_driver_value_handlers php_driver_bigint_handlers;

static HashTable *
php_driver_bigint_gc(CASS_COMPAT_OBJECT_HANDLER_TYPE *object, zval **table, int *n)
{
  *table = NULL;
  *n = 0;
  return zend_std_get_properties(object);
}

static HashTable *
php_driver_bigint_properties(CASS_COMPAT_OBJECT_HANDLER_TYPE *object)
{
  zval type;
  zval value;

  php_driver_numeric *self = CASS_COMPAT_GET_NUMERIC(object);
  HashTable         *props = zend_std_get_properties(object);

  type = php_driver_type_scalar(CASS_VALUE_TYPE_BIGINT);
  zend_hash_str_update(props, "type", strlen("type"), &(type));


  to_string(&(value), self);
  zend_hash_str_update(props, "value", strlen("value"), &(value));

  return props;
}

static int
php_driver_bigint_compare(zval *obj1, zval *obj2)
{
  php_driver_numeric *bigint1 = NULL;
  php_driver_numeric *bigint2 = NULL;

  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  bigint1 = PHP_DRIVER_GET_NUMERIC(obj1);
  bigint2 = PHP_DRIVER_GET_NUMERIC(obj2);

  if (bigint1->data.bigint.value == bigint2->data.bigint.value)
    return 0;
  else if (bigint1->data.bigint.value < bigint2->data.bigint.value)
    return -1;
  else
    return 1;
}

static unsigned
php_driver_bigint_hash_value(zval *obj)
{
  php_driver_numeric *self = PHP_DRIVER_GET_NUMERIC(obj);
  return (unsigned)(self->data.bigint.value ^ (self->data.bigint.value >> 32));
}

static int
php_driver_bigint_cast(CASS_COMPAT_OBJECT_HANDLER_TYPE *object, zval *retval, int type)
{
  php_driver_numeric *self = CASS_COMPAT_GET_NUMERIC(object);

  switch (type) {
  case IS_LONG:
      return to_long(retval, self);
  case IS_DOUBLE:
      return to_double(retval, self);
  case IS_STRING:
      return to_string(retval, self);
  default:
     return FAILURE;
  }
}

static void
php_driver_bigint_free(zend_object *object)
{
  php_driver_numeric *self = php_driver_numeric_object_fetch(object);;

  zend_object_std_dtor(&self->zval);

}

static zend_object *
php_driver_bigint_new(zend_class_entry *ce)
{
  php_driver_numeric *self =
      CASS_ZEND_OBJECT_ECALLOC(numeric, ce);

  self->type = PHP_DRIVER_BIGINT;

  CASS_ZEND_OBJECT_INIT_EX(numeric, bigint, self, ce);
}

void php_driver_define_Bigint()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\Bigint", php_driver_bigint_methods);
  php_driver_bigint_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_bigint_ce, 2, php_driver_value_ce, php_driver_numeric_ce);
  php_driver_bigint_ce->ce_flags     |= ZEND_ACC_FINAL;
  php_driver_bigint_ce->create_object = php_driver_bigint_new;

  memcpy(&php_driver_bigint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_bigint_handlers.std.get_properties  = php_driver_bigint_properties;
  php_driver_bigint_handlers.std.get_gc          = php_driver_bigint_gc;
  CASS_COMPAT_SET_COMPARE_HANDLER(php_driver_bigint_handlers.std, php_driver_bigint_compare);
  php_driver_bigint_handlers.std.cast_object     = php_driver_bigint_cast;

  php_driver_bigint_handlers.hash_value = php_driver_bigint_hash_value;
  php_driver_bigint_handlers.std.clone_obj = NULL;
}
