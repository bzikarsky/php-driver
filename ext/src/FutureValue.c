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

zend_class_entry *php_driver_future_value_ce = NULL;

PHP_METHOD(FutureValue, get)
{
  zval *timeout = NULL;
  php_driver_future_value *self = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &timeout) == FAILURE)
    return;

  self = PHP_DRIVER_GET_FUTURE_VALUE(getThis());

  if (!Z_ISUNDEF(self->value)) {
    RETURN_ZVAL(&(self->value), 1, 0);
  }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_timeout, 0, ZEND_RETURN_VALUE, 0)
  ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_future_value_methods[] = {
  PHP_ME(FutureValue, get, arginfo_timeout, ZEND_ACC_PUBLIC)
  PHP_FE_END
};

static zend_object_handlers php_driver_future_value_handlers;

static int
php_driver_future_value_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_future_value_free(zend_object *object)
{
  php_driver_future_value *self =
          php_driver_future_value_object_fetch(object);;

  CASS_ZVAL_MAYBE_DESTROY(self->value);

  zend_object_std_dtor(&self->zval);

}

static zend_object *
php_driver_future_value_new(zend_class_entry *ce)
{
  php_driver_future_value *self =
      CASS_ZEND_OBJECT_ECALLOC(future_value, ce);

  ZVAL_UNDEF(&(self->value));

  CASS_ZEND_OBJECT_INIT(future_value, self, ce);
}

void php_driver_define_FutureValue()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\FutureValue", php_driver_future_value_methods);
  php_driver_future_value_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_future_value_ce, 1, php_driver_future_ce);
  php_driver_future_value_ce->ce_flags     |= ZEND_ACC_FINAL;
  php_driver_future_value_ce->create_object = php_driver_future_value_new;

  memcpy(&php_driver_future_value_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  CASS_COMPAT_SET_COMPARE_HANDLER(php_driver_future_value_handlers, php_driver_future_value_compare);
  php_driver_future_value_handlers.clone_obj = NULL;
}
