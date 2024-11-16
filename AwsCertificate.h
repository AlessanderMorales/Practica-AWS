// AWS_certificates.h
#ifndef AWS_CERTIFICATE_H
#define AWS_CERTIFICATE_H

// Certificado del cliente
const char CERTIFICATE[] = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----

)KEY";

// Clave privada del cliente
const char PRIVATE_KEY[] = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)KEY";

// Certificado de la CA (Amazon Root CA 1)
const char AMAZON_ROOT_CA1[] = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)KEY";

#endif
