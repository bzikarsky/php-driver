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
#include "util/types.h"
#include "util/ref.h"

#include "DefaultColumn.h"

zend_class_entry *php_driver_default_column_ce = NULL;

zval
php_driver_create_column(php_driver_ref *schema,
                         const CassColumnMeta *meta)
{
  zval result;
  php_driver_column *column;
  const char *name;
  size_t name_length;
  const CassValue *value;

  ZVAL_UNDEF(&(result));


  object_init_ex(&(result), php_driver_default_column_ce);

  column = PHP_DRIVER_GET_COLUMN(&(result));
  column->schema = php_driver_add_ref(schema);
  column->meta   = meta;

  cass_column_meta_name(meta, &name, &name_length);

  ZVAL_STRINGL(&(column->name), name, name_length);

  value = cass_column_meta_field_by_name(meta, "validator");
  if (value) {
    const char *validator;
    size_t validator_length;

    ASSERT_SUCCESS_BLOCK(cass_value_get_string(value,
                                               &validator,
                                               &validator_length),
     zval_ptr_dtor(&result);
     ZVAL_UNDEF(&(result));
     return result;
    );

    if (php_driver_parse_column_type(validator, validator_length,
                                        &column->reversed, &column->frozen,
                                        &column->type) == FAILURE) {
      zval_ptr_dtor(&result);
      ZVAL_UNDEF(&(result));
      return result;
    }
  } else {
    const CassDataType *data_type = cass_column_meta_data_type(meta);
    if (data_type) {
      const char *clustering_order;
      size_t clustering_order_length;
      column->type = php_driver_type_from_data_type(data_type);

#if CURRENT_CPP_DRIVER_VERSION > CPP_DRIVER_VERSION(2, 2, 0)
      column->frozen = cass_data_type_is_frozen(data_type);
#else
      column->frozen = 0;
#endif

      value = cass_column_meta_field_by_name(meta, "clustering_order");
      if (!value) {
        zend_throw_exception_ex(php_driver_runtime_exception_ce, 0,
                                "Unable to get column field \"clustering_order\"");
        zval_ptr_dtor(&result);
        ZVAL_UNDEF(&(result));
        return result;
      }

     ASSERT_SUCCESS_BLOCK(cass_value_get_string(value,
                                                &clustering_order,
                                                &clustering_order_length),
        zval_ptr_dtor(&result);
        ZVAL_UNDEF(&(result));
        return result;
      );
      column->reversed =
          strncmp(clustering_order, "desc", clustering_order_length) == 0 ? 1 : 0;
    }
  }

  return result;
}


PHP_METHOD(DefaultColumn, name)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  RETURN_ZVAL(&(self->name), 1, 0);
}

PHP_METHOD(DefaultColumn, type)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  if (Z_ISUNDEF(self->type)) {
    RETURN_NULL();
  }

  RETURN_ZVAL(&(self->type), 1, 0);
}

PHP_METHOD(DefaultColumn, isReversed)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  RETURN_BOOL(self->reversed);
}

PHP_METHOD(DefaultColumn, isStatic)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self  = PHP_DRIVER_GET_COLUMN(getThis());

  RETURN_BOOL(cass_column_meta_type(self->meta) == CASS_COLUMN_TYPE_STATIC);
}

PHP_METHOD(DefaultColumn, isFrozen)
{
  php_driver_column *self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  RETURN_BOOL(self->frozen);
}

PHP_METHOD(DefaultColumn, indexName)
{
  php_driver_column *self;
  zval value;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  php_driver_get_column_field(self->meta, "index_name", &value);
  RETURN_ZVAL(&(value), 0, 1);
}

PHP_METHOD(DefaultColumn, indexOptions)
{
  php_driver_column *self;
  zval value;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_GET_COLUMN(getThis());

  php_driver_get_column_field(self->meta, "index_options", &value);
  RETURN_ZVAL(&(value), 0, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_none, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

static zend_function_entry php_driver_default_column_methods[] = {
  PHP_ME(DefaultColumn, name,         arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultColumn, type,         arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultColumn, isReversed,   arginfo_none, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
  PHP_ME(DefaultColumn, isStatic,     arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultColumn, isFrozen,     arginfo_none, ZEND_ACC_PUBLIC)
  PHP_ME(DefaultColumn, indexName,    arginfo_none, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
  PHP_ME(DefaultColumn, indexOptions, arginfo_none, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
  PHP_FE_END
};

static zend_object_handlers php_driver_default_column_handlers;

static HashTable *
php_driver_type_default_column_gc(CASS_COMPAT_OBJECT_HANDLER_TYPE *object, zval **table, int *n)
{
  *table = NULL;
  *n = 0;
  return zend_std_get_properties(object);
}

static int
php_driver_default_column_compare(zval *obj1, zval *obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_default_column_free(zend_object *object)
{
  php_driver_column *self = php_driver_column_object_fetch(object);;

  CASS_ZVAL_MAYBE_DESTROY(self->name);
  CASS_ZVAL_MAYBE_DESTROY(self->type);

  if (self->schema) {
    php_driver_del_ref(&self->schema);
    self->schema = NULL;
  }
  self->meta = NULL;

  zend_object_std_dtor(&self->zval);

}

static zend_object *
php_driver_default_column_new(zend_class_entry *ce)
{
  php_driver_column *self =
      CASS_ZEND_OBJECT_ECALLOC(column, ce);

  self->reversed = 0;
  self->frozen   = 0;
  self->schema   = NULL;
  self->meta     = NULL;
  ZVAL_UNDEF(&(self->name));
  ZVAL_UNDEF(&(self->type));

  CASS_ZEND_OBJECT_INIT_EX(column, default_column, self, ce);
}

void php_driver_define_DefaultColumn()
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, PHP_DRIVER_NAMESPACE "\\DefaultColumn", php_driver_default_column_methods);
  php_driver_default_column_ce = zend_register_internal_class(&ce);
  zend_class_implements(php_driver_default_column_ce, 1, php_driver_column_ce);
  php_driver_default_column_ce->ce_flags     |= ZEND_ACC_FINAL;
  php_driver_default_column_ce->create_object = php_driver_default_column_new;

  memcpy(&php_driver_default_column_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_default_column_handlers.get_gc          = php_driver_type_default_column_gc;
  CASS_COMPAT_SET_COMPARE_HANDLER(php_driver_default_column_handlers, php_driver_default_column_compare);
  php_driver_default_column_handlers.clone_obj = NULL;
}
