/* soapSrbDownloadProxy.h
   Generated by gSOAP 2.7.9b from SrbStorageWS.h
   Copyright(C) 2000-2006, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef soapSrbDownload_H
#define soapSrbDownload_H
#include "soapH.h"
class SrbDownload
{   public:
	struct soap *soap;
	const char *endpoint;
	SrbDownload()
	{ soap = soap_new(); endpoint = "http://localhost:80"; if (soap && !soap->namespaces) { static const struct Namespace namespaces[] = 
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns3", "http://www.w3.org/2004/08/xop/include", NULL, NULL},
	{"ns4", "http://www.w3.org/2004/11/xmlmime", NULL, NULL},
	{"ns6", "http://localhost:80/SrbList.wsdl", NULL, NULL},
	{"ns1", "SrbList", NULL, NULL},
	{"ns7", "http://localhost:80/SrbDownload.wsdl", NULL, NULL},
	{"ns2", "urn:SrbDownload", NULL, NULL},
	{"ns8", "http://localhost:80/SrbUpload.wsdl", NULL, NULL},
	{"ns5", "http://tempuri.org/ns.xsd", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	soap->namespaces = namespaces; } };
	virtual ~SrbDownload() { if (soap) { soap_destroy(soap); soap_end(soap); soap_free(soap); } };
	virtual int __ns7__SrbDownload(_ns2__SrbDownload *ns2__SrbDownload, struct __ns7__SrbDownloadResponse &_param_1) { return soap ? soap_call___ns7__SrbDownload(soap, endpoint, NULL, ns2__SrbDownload, _param_1) : SOAP_EOM; };
};
#endif