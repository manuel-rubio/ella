Ella Web Server
===============

**DISCLAIMER** This project was created in 2008 only to check some aspects for the web server development. This web server isn't functional and it's discontinued.

¿Qué es esto?
-------------

Ella Web Server (ews) es un sistema pensado como *servidor HTTP* o *servidor web*, que pueda ampliarse mediante módulos y tenga las funcionalidades completas de los estándares HTTP que se ven reflejados en los RFCs correspondientes a los entornos web.

La licencia del software es [GPL](http://www.viti.es/gnu/licenses/gpl.html) y toda la documentación sobre el servidor es [FDL](http://curso-sobre.berlios.de/gfdles/gfdles.html).

¿Por qué otro servidor web?
---------------------------

En software libre existen una serie de servidores web (Apahce, Cherokee, Nginx, thttpd, lighttp...), cada uno con sus particularidades específicas. La motivación de crear este servidor ha sido por no encontrar ciertas configuraciones en los otros, limitaciones en algunos aspectos y poca flexibilidad.

Lo que *ews* pretende aportar al panorama de servidores web, es:

* Servidor **orientado a los módulos**, que sea fácil realizar extensiones para él. El núcleo del servidor es básico y flexible. Permite cargar cualquier módulo realizado con la librería _ews_ y agregarlo como funcionalidad para las peticiones HTTP, como comandos de consola, configuración, logs, etc.
* **Configuración flexible**, la configuración es también un módulo que puede cambiarse para usar otro, con lo que se puede realizar la configuración en INI, XML, MySQL, LDAP, etc.
* Información sobre las acciones del servidor en **multi-log**. Esto es que se pueden configurar varios sistemas para la recogida de logs. También es modular, de modo que se pueden tener los logs en: Syslog, consola, fichero personalizado, MySQL, etc.
* Uso de una **consola** para interacción con el sistema. Al igual que tienen otros sistemas como Asterisk, usamos una consola para introducir comandos y ver los logs en tiempo real, cargar y descargar módulos, cambiar aspectos de la configuración, etc.
* Soporte de **protocolo en módulos**. Esto quiere decir que se puede emplear el módulo _http10_ para atender las peticiones (HTTP versión 1.0), un módulo _http11_, u otro modificado a nuestro gusto o necesidades.

Compilar y Probar
-----------------

Simplemente:

```
./configure
make
make install
```

Para más información lee el fichero INSTALL.

Enjoy!
