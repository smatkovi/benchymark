# extern/

Die Kopfdateien von Khronos fuer OpenGL ES 2 und 3, unveraendert von
<https://registry.khronos.org/OpenGL/api/> (Lizenz MIT/Apache-2.0, steht im
Kopf jeder Datei).

Sie liegen im Baum, damit der Bau keine `android-headers` braucht: auf
SailfishOS kommen die GLES-Kopfdateien sonst aus `libhybris-libGLESv2-devel`,
und dieses Paket zieht Abhaengigkeiten nach, die auf dem Geraet die Anzeige
auseinandernehmen koennen. Gebunden wird trotzdem gegen die echten
Bibliotheken des Geraets.
