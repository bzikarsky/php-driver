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
#include "util/result.h"
#include "util/ref.h"
#include "util/types.h"

#include "DefaultFunction.h"

zend_class_entry *php_driver_default_aggregate_ce = NULL;

PHP_METHOD(DefaultAggregate, name)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());

  RETURN_ZVAL(&(self->signature), 1, 0);
}

PHP_METHOD(DefaultAggregate, simpleName)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->simple_name)) {
    const char *name;
    size_t name_length;
    cass_aggregate_meta_name(self->meta, &name, &name_length);

    ZVAL_STRINGL(&(self->simple_name), name, name_length);
  }

  RETURN_ZVAL(&(self->simple_name), 1, 0);
}

PHP_METHOD(DefaultAggregate, argumentTypes)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->argument_types)) {
    size_t i, count = cass_aggregate_meta_argument_count(self->meta);

    array_init(&(self->argument_types));
    for (i = 0; i < count; ++i) {
      const CassDataType* data_type = cass_aggregate_meta_argument_type(self->meta, i);
      if (data_type) {
        zval type = php_driver_type_from_data_type(data_type);
        if (!Z_ISUNDEF(type)) {
          add_next_index_zval(&(self->argument_types),
                              &(type));
        }
      }
    }
  }

  RETURN_ZVAL(&(self->argument_types), 1, 0);
}

PHP_METHOD(DefaultAggregate, stateFunction)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->state_function)) {
    const CassFunctionMeta* function = cass_aggregate_meta_state_func(self->meta);
    if (!function) {
      return;
    }
    self->state_function =
        php_driver_create_function(self->schema, function);
  }

  RETURN_ZVAL(&(self->state_function), 1, 0);
}

PHP_METHOD(DefaultAggregate, finalFunction)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->final_function)) {
    const CassFunctionMeta* function = cass_aggregate_meta_final_func(self->meta);
    if (!function) {
      return;
    }
    self->final_function =
        php_driver_create_function(self->schema, function);
  }

  RETURN_ZVAL(&(self->final_function), 1, 0);
}

PHP_METHOD(DefaultAggregate, initialCondition)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->initial_condition)) {
    const CassValue *value = cass_aggregate_meta_init_cond(self->meta);
    const CassDataType *data_type = NULL;
    if (!value) {
      return;
    }
    data_type = cass_value_data_type(value);
    if (!data_type) {
      return;
    }
    php_driver_value(value, data_type, &self->initial_condition);
  }

  RETURN_ZVAL(&(self->initial_condition), 1, 0);
}

PHP_METHOD(DefaultAggregate, stateType)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->state_type)) {
    const CassDataType* data_type = cass_aggregate_meta_state_type(self->meta);
    if (!data_type) {
      return;
    }
    self->state_type = php_driver_type_from_data_type(data_type);
  }

  RETURN_ZVAL(&(self->state_type), 1, 0);
}

PHP_METHOD(DefaultAggregate, returnType)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  if (Z_ISUNDEF(self->return_type)) {
    const CassDataType* data_type = cass_aggregate_meta_return_type(self->meta);
    if (!data_type) {
      return;
    }
    self->return_type = php_driver_type_from_data_type(data_type);
  }

  RETURN_ZVAL(&(self->return_type), 1, 0);
}

PHP_METHOD(DefaultAggregate, signature)
{
  php_driver_aggregate *self;

  if (zend_parse_parameters_none() == FAILURE)
    return;

  self = PHP_DRIVER_GET_AGGREGATE(getThis());
  RETURN_ZVAL(&(self->signature), 1, 0);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_default_aggregate_methods[] = {
  PHP_ME(DefaultAggregate, name, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultAggregate, simpleName, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultAggregate, argumentTypes, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultAggregate, stateFunction, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultAggregate, finalFunction, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultAggregate, initialCondition, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultAggregate, stateType, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultAggregate, returnType, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultAggregate, signature, arginfo_none, ZEND_ACC_PUBLIC)
  PHP_FE_END
};

static zend_object_handlers php_driver_default_aggregate_handlers;

static HashTable *
php_driver_type_default_aggregate_gc(CASS_COMPAT_OBJECT_HANDLER_TYPE *object, zval **table, int *n)
{
  *table = NULL;
  *n = 0;
  return zend_std_get_properties(object);
}

static int
php_driver_default_aggregate_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_default_aggregate_free(zend_object *object)
{
  php_driver_aggregate *self = php_driver_aggregate_object_fetch(object);;

  CASS_ZVAL_MAYBE_DESTROY(self->simple_name);
  CASS_ZVAL_MAYBE_DESTROY(self->argument_types);
  CASS_ZVAL_MAYBE_DESTROY(self->state_function);
  CASS_ZVAL_MAYBE_DESTROY(self->final_function);
  CASS_ZVAL_MAYBE_DESTROY(self->initial_condition);
  CASS_ZVAL_MAYBE_DESTROY(self->state_type);
  CASS_ZVAL_MAYBE_DESTROY(self->return_type);
  CASS_ZVAL_MAYBE_DESTROY(self->signature);

  if (self->schema) {
    php_driver_del_ref(&self->schema);
    self->schema = NULL;
  }
  self->meta = NULL;

  zend_object_std_dtor(&self->zval);

}

static zend_object *
php_driver_default_aggregate_new(zend_class_entry *ce)
{
  php_driver_aggregate *self =
      CASS_ZEND_OBJECT_ECALLOC(aggregate, ce);

  ZVAL_UNDEF(&(self->simple_name));
  ZVAL_UNDEF(&(self->argument_types));
  ZVAL_UNDEF(&(self->state_function));
  ZVAL_UNDEF(&(self->final_function));
  ZVAL_UNDEF(&(self->initial_condition));
  ZVAL_UNDEF(&(self->state_type));
  ZVAL_UNDEF(&(self->return_type));
  ZVAL_UNDEF(&(self->signature));

  self->schema = NULL;
  self->meta = NULL;

  CASS_ZEND_OBJECT_INIT_EX(aggregate, default_aggregate, self, ce);
}

void php_driver_define_DefaultAggregate()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\DefaultAggregate", php_driver_default_aggregate_methods);
  php_driver_default_aggregate_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_default_aggregate_ce, 1, php_driver_aggregate_ce);
  php_driver_default_aggregate_ce->ce_flags     |= ZEND_ACC_FINAL;
  php_driver_default_aggregate_ce->create_object = php_driver_default_aggregate_new;

  memcpy(&php_driver_default_aggregate_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_default_aggregate_handlers.get_gc          = php_driver_type_default_aggregate_gc;
  CASS_COMPAT_SET_COMPARE_HANDLER(php_driver_default_aggregate_handlers, php_driver_default_aggregate_compare);
  php_driver_default_aggregate_handlers.clone_obj = NULL;
}
