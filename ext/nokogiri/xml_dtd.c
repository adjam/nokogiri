#include <xml_dtd.h>

static void notation_copier(void *payload, void *data, xmlChar *name)
{
  VALUE hash = (VALUE)data;
  VALUE klass = rb_const_get(mNokogiriXml, rb_intern("Notation"));

  xmlNotationPtr c_notation = (xmlNotationPtr)payload;

  VALUE notation = rb_funcall(klass, rb_intern("new"), 3,
      c_notation->name ? NOKOGIRI_STR_NEW2(c_notation->name, "UTF-8") : Qnil,
      c_notation->PublicID ? NOKOGIRI_STR_NEW2(c_notation->PublicID, "UTF-8") : Qnil,
      c_notation->SystemID ? NOKOGIRI_STR_NEW2(c_notation->SystemID, "UTF-8") : Qnil);

  rb_hash_aset(hash, NOKOGIRI_STR_NEW2(name, "UTF-8"),notation);
}

static void element_copier(void *_payload, void *data, xmlChar *name)
{
  VALUE hash = (VALUE)data;
  xmlNodePtr payload = (xmlNodePtr)_payload;

  VALUE element = Nokogiri_wrap_xml_node(Qnil, payload);

  rb_hash_aset(hash, NOKOGIRI_STR_NEW2(name, payload->doc->encoding), element);
}

/*
 * call-seq:
 *   entities
 *
 * Get a hash of the elements for this DTD.
 */
static VALUE entities(VALUE self)
{
  xmlDtdPtr dtd;
  Data_Get_Struct(self, xmlDtd, dtd);

  if(!dtd->entities) return Qnil;

  VALUE hash = rb_hash_new();

  xmlHashScan((xmlHashTablePtr)dtd->entities, element_copier, (void *)hash);

  return hash;
}

/*
 * call-seq:
 *   notations
 *
 * Get a hash of the notations for this DTD.
 */
static VALUE notations(VALUE self)
{
  xmlDtdPtr dtd;
  Data_Get_Struct(self, xmlDtd, dtd);

  if(!dtd->notations) return Qnil;

  VALUE hash = rb_hash_new();

  xmlHashScan((xmlHashTablePtr)dtd->notations, notation_copier, (void *)hash);

  return hash;
}

/*
 * call-seq:
 *   attributes
 *
 * Get a hash of the attributes for this DTD.
 */
static VALUE attributes(VALUE self)
{
  xmlDtdPtr dtd;
  Data_Get_Struct(self, xmlDtd, dtd);

  if(!dtd->attributes) return Qnil;

  VALUE hash = rb_hash_new();

  xmlHashScan((xmlHashTablePtr)dtd->attributes, element_copier, (void *)hash);

  return hash;
}

/*
 * call-seq:
 *   elements
 *
 * Get a hash of the elements for this DTD.
 */
static VALUE elements(VALUE self)
{
  xmlDtdPtr dtd;
  Data_Get_Struct(self, xmlDtd, dtd);

  if(!dtd->elements) return Qnil;

  VALUE hash = rb_hash_new();

  xmlHashScan((xmlHashTablePtr)dtd->elements, element_copier, (void *)hash);

  return hash;
}

/*
 * call-seq:
 *   validate(document)
 *
 * Validate +document+ returning a list of errors
 */
static VALUE validate(VALUE self, VALUE document)
{
  xmlDocPtr doc;
  xmlDtdPtr dtd;

  Data_Get_Struct(self, xmlDtd, dtd);
  Data_Get_Struct(document, xmlDoc, doc);
  VALUE error_list      = rb_ary_new();

  xmlValidCtxtPtr ctxt = xmlNewValidCtxt();

  xmlSetStructuredErrorFunc((void *)error_list, Nokogiri_error_array_pusher);

  xmlValidateDtd(ctxt, doc, dtd);

  xmlSetStructuredErrorFunc(NULL, NULL);

  xmlFreeValidCtxt(ctxt);

  return error_list;
}

/*
 * call-seq:
 *   system_id
 *
 * Get the System ID for this DTD
 */
static VALUE system_id(VALUE self)
{
  xmlDtdPtr dtd;
  Data_Get_Struct(self, xmlDtd, dtd);

  if(!dtd->SystemID) return Qnil;

  return NOKOGIRI_STR_NEW2(dtd->SystemID, NULL);
}

/*
 * call-seq:
 *   external_id
 *
 * Get the External ID for this DTD
 */
static VALUE external_id(VALUE self)
{
  xmlDtdPtr dtd;
  Data_Get_Struct(self, xmlDtd, dtd);

  if(!dtd->ExternalID) return Qnil;

  return NOKOGIRI_STR_NEW2(dtd->ExternalID, NULL);
}

VALUE cNokogiriXmlDtd;

void init_xml_dtd()
{
  VALUE nokogiri = rb_define_module("Nokogiri");
  VALUE xml = rb_define_module_under(nokogiri, "XML");
  VALUE node = rb_define_class_under(xml, "Node", rb_cObject);

  /*
   * Nokogiri::XML::DTD wraps DTD nodes in an XML document
   */
  VALUE klass = rb_define_class_under(xml, "DTD", node);

  cNokogiriXmlDtd = klass;

  rb_define_method(klass, "notations", notations, 0);
  rb_define_method(klass, "elements", elements, 0);
  rb_define_method(klass, "entities", entities, 0);
  rb_define_method(klass, "validate", validate, 1);
  rb_define_method(klass, "attributes", attributes, 0);
  rb_define_method(klass, "system_id", system_id, 0);
  rb_define_method(klass, "external_id", external_id, 0);
}
