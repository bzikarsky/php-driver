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
#include "util/types.h"

#include "src/Type/Tuple.h"
#include "src/Type/UserType.h"

zend_class_entry *php_driver_type_ce = NULL;

#define XX_SCALAR_METHOD(name, value) PHP_METHOD(Type, name) \
{ \
  zval ztype; \
  if (zend_parse_parameters_none() == FAILURE) { \
    return; \
  } \
  ztype = php_driver_type_scalar(value); \
  RETURN_ZVAL(&(ztype), 1, 1); \
}

PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR_METHOD)
#undef XX_SCALAR_METHOD

PHP_METHOD(Type, collection)
{
  zval ztype;
  zval *value_type;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
                            &value_type, php_driver_type_ce) == FAILURE) {
    return;
  }

  if (!php_driver_type_validate(value_type, "type")) {
    return;
  }

  ztype  = php_driver_type_collection(value_type);
  Z_ADDREF_P(value_type);
  RETURN_ZVAL(&(ztype), 0, 1);
}

PHP_METHOD(Type, tuple)
{
  zval ztype;
  php_driver_type *type;
  zval *args = NULL;
  int argc = 0, i;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+",
                            &args, &argc) == FAILURE) {
    return;
  }

  for (i = 0; i < argc; ++i) {
    zval *sub_type = &(args[i]);
    if (!php_driver_type_validate(sub_type, "type")) {

      return;
    }
  }

  ztype = php_driver_type_tuple();
  type = PHP_DRIVER_GET_TYPE(&(ztype));

  for (i = 0; i < argc; ++i) {
    zval *sub_type = &(args[i]);
    if (php_driver_type_tuple_add(type, sub_type)) {
      Z_ADDREF_P(sub_type);
    } else {
      break;
    }
  }


  RETURN_ZVAL(&(ztype), 0, 1);
}

PHP_METHOD(Type, userType)
{
  zval ztype;
  php_driver_type *type;
  zval *args = NULL;
  int argc = 0, i;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+",
                            &args, &argc) == FAILURE) {
    return;
  }

  if (argc % 2 == 1) {
    zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                            "Not enough name/type pairs, user types can only be created " \
                            "from an even number of name/type pairs, where each odd " \
                            "argument is a name and each even argument is a type, " \
                            "e.g userType(name, type, name, type, name, type)");

    return;
  }

  for (i = 0; i < argc; i += 2) {
    zval *name = &(args[i]);
    zval *sub_type = &(args[i + 1]);
    if (Z_TYPE_P(name) != IS_STRING) {
      zend_throw_exception_ex(php_driver_invalid_argument_exception_ce, 0,
                              "Argument %d is not a string", i + 1);

      return;
    }
    if (!php_driver_type_validate(sub_type, "type")) {

      return;
    }
  }

  ztype = php_driver_type_user_type();
  type = PHP_DRIVER_GET_TYPE(&(ztype));

  for (i = 0; i < argc; i += 2) {
    zval *name = &(args[i]);
    zval *sub_type = &(args[i + 1]);
    if (php_driver_type_user_type_add(type,
                                         Z_STRVAL_P(name), Z_STRLEN_P(name),
                                         sub_type)) {
      Z_ADDREF_P(sub_type);
    } else {
      break;
    }
  }



  RETURN_ZVAL(&(ztype), 0, 1);
}

PHP_METHOD(Type, set)
{
  zval ztype;
  zval *value_type;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
                            &value_type, php_driver_type_ce) == FAILURE) {
    return;
  }

  if (!php_driver_type_validate(value_type, "type")) {
    return;
  }

  ztype = php_driver_type_set(value_type);
  Z_ADDREF_P(value_type);
  RETURN_ZVAL(&(ztype), 0, 1);
}

PHP_METHOD(Type, map)
{
  zval ztype;
  zval *key_type;
  zval *value_type;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "OO",
                            &key_type, php_driver_type_ce,
                            &value_type, php_driver_type_ce) == FAILURE) {
    return;
  }

  if (!php_driver_type_validate(key_type, "keyType")) {
    return;
  }

  if (!php_driver_type_validate(value_type, "valueType")) {
    return;
  }

  ztype = php_driver_type_map(key_type, value_type);
  Z_ADDREF_P(key_type);
  Z_ADDREF_P(value_type);
  RETURN_ZVAL(&(ztype), 0, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_types, 0, ZEND_RETURN_VALUE, 0)
  ZEND_ARG_INFO(0, types)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_type, 0, ZEND_RETURN_VALUE, 1)
  PHP_DRIVER_NAMESPACE_ZEND_ARG_OBJ_INFO(0, type, Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_map, 0, ZEND_RETURN_VALUE, 2)
  PHP_DRIVER_NAMESPACE_ZEND_ARG_OBJ_INFO(0, keyType,   Type, 0)
  PHP_DRIVER_NAMESPACE_ZEND_ARG_OBJ_INFO(0, valueType, Type, 0)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_type_methods[] = {
  PHP_ABSTRACT_ME(Type, name,       arginfo_none)
  PHP_ABSTRACT_ME(Type, __toString, arginfo_none)

#define XX_SCALAR_METHOD(name, _) PHP_ME(Type, name, arginfo_none, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
  PHP_DRIVER_SCALAR_TYPES_MAP(XX_SCALAR_METHOD)
#undef XX_SCALAR_METHOD
  PHP_ME(Type, collection, arginfo_type,  ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
  PHP_ME(Type, set,        arginfo_type,  ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
  PHP_ME(Type, map,        arginfo_map,   ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
  PHP_ME(Type, tuple,      arginfo_types, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
  PHP_ME(Type, userType,   arginfo_types, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
  PHP_FE_END
};

void php_driver_define_Type()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\Type", php_driver_type_methods);
  php_driver_type_ce = zend_register_internal_class(&ce);
  php_driver_type_ce->ce_flags |= ZEND_ACC_ABSTRACT;
}
