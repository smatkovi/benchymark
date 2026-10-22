<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1">
<context>
    <name></name>
    <message id="id-stage-benchy-light">
        <location filename="../qml/pages/MainPage.qml" line="29"/>
        <source>Benchy light</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-benchy-light">
        <location filename="../qml/pages/MainPage.qml" line="31"/>
        <source>Two 3DBenchy wireframes turning, each drawn ten times on its spot: the lines are rotated and projected on the CPU, then stroked by the GPU.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-benchy-heavy">
        <location filename="../qml/pages/MainPage.qml" line="33"/>
        <source>Benchy heavy</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-benchy-heavy">
        <location filename="../qml/pages/MainPage.qml" line="35"/>
        <source>Six boats, each drawn twenty times: 120 wireframes every frame, much more geometry than the picture shows.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-glow">
        <location filename="../qml/pages/MainPage.qml" line="37"/>
        <source>Benchy glow</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-glow">
        <location filename="../qml/pages/MainPage.qml" line="39"/>
        <source>A Benchy with a pulsing golden halo: a blur over the whole screen every frame, the cost of soft effects.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-shader-light">
        <location filename="../qml/pages/MainPage.qml" line="41"/>
        <source>Shader light</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-shader-light">
        <location filename="../qml/pages/MainPage.qml" line="43"/>
        <source>A drifting noise field computed for every pixel: GPU arithmetic that grows with the screen size.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-shader-heavy">
        <location filename="../qml/pages/MainPage.qml" line="45"/>
        <source>Shader heavy</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-shader-heavy">
        <location filename="../qml/pages/MainPage.qml" line="47"/>
        <source>The same idea warped twice over, about thirty noise lookups per pixel: the heaviest GPU load here.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-vulkan">
        <location filename="../qml/pages/MainPage.qml" line="49"/>
        <location filename="../qml/stages/Vulkan.qml" line="79"/>
        <location filename="../qml/stages/Vulkan.qml" line="81"/>
        <location filename="../qml/stages/Vulkan.qml" line="86"/>
        <source>Vulkan shader</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-vulkan">
        <location filename="../qml/pages/MainPage.qml" line="51"/>
        <source>The same shader through Vulkan, drawn without a window into memory and timed by the GPU itself: what the other road costs for the same picture.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-overdraw">
        <location filename="../qml/pages/MainPage.qml" line="53"/>
        <source>Overdraw</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-overdraw">
        <location filename="../qml/pages/MainPage.qml" line="55"/>
        <source>72 large translucent squares stacked and turning: every pixel is blended many times, raw fill rate.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-cascade">
        <location filename="../qml/pages/MainPage.qml" line="57"/>
        <source>Cascade</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-cascade">
        <location filename="../qml/pages/MainPage.qml" line="59"/>
        <source>A thousand rounded squares in two breathing, counter-rotating rings: many moving items under transformed parents.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-icons">
        <location filename="../qml/pages/MainPage.qml" line="61"/>
        <source>Icon storm</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-icons">
        <location filename="../qml/pages/MainPage.qml" line="63"/>
        <source>Rockets and flashes on spiral arms, each icon drawn three times: many small images moving every frame.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-scale">
        <location filename="../qml/pages/MainPage.qml" line="65"/>
        <source>Text scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-scale">
        <location filename="../qml/pages/MainPage.qml" line="67"/>
        <source>Numbers growing and shrinking by a scale transform: text drawn once and only resized, the cheap way.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-reraster">
        <location filename="../qml/pages/MainPage.qml" line="69"/>
        <source>Text reraster</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-reraster">
        <location filename="../qml/pages/MainPage.qml" line="71"/>
        <source>The same picture made by changing the font size, so every glyph is drawn anew each frame: what animated text size costs.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-cpu">
        <location filename="../qml/pages/MainPage.qml" line="73"/>
        <location filename="../qml/stages/CpuPrime.qml" line="22"/>
        <source>CPU prime</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-cpu">
        <location filename="../qml/pages/MainPage.qml" line="75"/>
        <source>All cores count primes as sysbench does; every finished count lights a cell of an Ulam spiral if its number is prime.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-cpu-single">
        <location filename="../qml/pages/MainPage.qml" line="77"/>
        <location filename="../qml/stages/CpuPrimeSingle.qml" line="11"/>
        <source>CPU single core</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-cpu-single">
        <location filename="../qml/pages/MainPage.qml" line="79"/>
        <source>The same prime counting on one core only: next to the all core figure it shows what a single core does.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-memory">
        <location filename="../qml/pages/MainPage.qml" line="81"/>
        <location filename="../qml/stages/Memory.qml" line="112"/>
        <source>Memory throughput</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-memory">
        <location filename="../qml/pages/MainPage.qml" line="83"/>
        <source>Copies memory between two 32 MB buffers as fast as it can: how quickly the phone moves data in RAM.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-million">
        <location filename="../qml/pages/MainPage.qml" line="85"/>
        <location filename="../qml/stages/OneMillion.qml" line="84"/>
        <source>One million</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-million">
        <location filename="../qml/pages/MainPage.qml" line="87"/>
        <source>JavaScript counts to one million, over and over: how fast the app&apos;s script engine runs a plain loop.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-storage">
        <location filename="../qml/pages/MainPage.qml" line="90"/>
        <source>Storage performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-about-storage">
        <location filename="../qml/pages/MainPage.qml" line="92"/>
        <source>Writes 256 MB in small blocks past the cache and reads it back. Off by default: it wears the flash a little every run.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-battery">
        <location filename="../qml/pages/MainPage.qml" line="949"/>
        <source>Battery</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-clocks">
        <location filename="../qml/pages/MainPage.qml" line="1028"/>
        <source>Clocks</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-brightness">
        <location filename="../qml/pages/MainPage.qml" line="1174"/>
        <source>brightness</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-set-by-run">
        <location filename="../qml/pages/MainPage.qml" line="1176"/>
        <source>set by the run</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-charging">
        <location filename="../qml/pages/MainPage.qml" line="1178"/>
        <source>charging</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-run-hour">
        <location filename="../qml/pages/MainPage.qml" line="1194"/>
        <source>Run one hour</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-run-loop">
        <location filename="../qml/pages/MainPage.qml" line="1196"/>
        <source>Run in loop</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-run-once">
        <location filename="../qml/pages/MainPage.qml" line="1192"/>
        <source>Run once</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-start">
        <location filename="../qml/pages/MainPage.qml" line="590"/>
        <source>Start</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-next">
        <location filename="../qml/pages/MainPage.qml" line="650"/>
        <source>Next: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-tap-to-stop">
        <location filename="../qml/pages/MainPage.qml" line="698"/>
        <source>Tap to stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-loop-short">
        <location filename="../qml/pages/MainPage.qml" line="792"/>
        <source>loop</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-col-stage">
        <location filename="../qml/pages/MainPage.qml" line="811"/>
        <source>Stage</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-col-avg">
        <location filename="../qml/pages/MainPage.qml" line="819"/>
        <location filename="../qml/pages/MainPage.qml" line="1168"/>
        <source>avg</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-col-min">
        <location filename="../qml/pages/MainPage.qml" line="826"/>
        <source>min</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-col-worst">
        <location filename="../qml/pages/MainPage.qml" line="833"/>
        <source>worst ms</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-no-direct">
        <location filename="../qml/pages/MainPage.qml" line="874"/>
        <source>* without O_DIRECT: the read may come from the cache</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-temperature">
        <location filename="../qml/pages/MainPage.qml" line="889"/>
        <source>Temperature</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-share-image">
        <location filename="../qml/pages/MainPage.qml" line="1099"/>
        <source>Share image</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-share-json">
        <location filename="../qml/pages/MainPage.qml" line="1104"/>
        <source>Share JSON</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-back">
        <location filename="../qml/pages/MainPage.qml" line="1111"/>
        <source>Back</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-share-results">
        <location filename="../qml/pages/MainPage.qml" line="1122"/>
        <source>Share results</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-hint-once">
        <location filename="../qml/pages/MainPage.qml" line="1200"/>
        <source>For comparable results</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-hint-hour">
        <location filename="../qml/pages/MainPage.qml" line="1202"/>
        <source>A defined battery and temperature test, ends at 1:00:00. The display runs at 50 % brightness with automatic brightness off, and is set back afterwards.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-hint-loop">
        <location filename="../qml/pages/MainPage.qml" line="1204"/>
        <source>Until you tap: overheating and throttling diagnosis</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-not-throttled">
        <location filename="../qml/pages/MainPage.qml" line="1214"/>
        <source>not throttled</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-throttled">
        <location filename="../qml/pages/MainPage.qml" line="1216"/>
        <source>throttled %1 % of the run, first at %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-peak">
        <location filename="../qml/pages/MainPage.qml" line="1225"/>
        <source>peak</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-writing">
        <location filename="../qml/stages/Storage.qml" line="72"/>
        <location filename="../qml/stages/Storage.qml" line="80"/>
        <source>Writing</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-reading">
        <location filename="../qml/stages/Storage.qml" line="72"/>
        <location filename="../qml/stages/Storage.qml" line="82"/>
        <source>Reading</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-stage-storage-rw">
        <location filename="../qml/stages/Storage.qml" line="87"/>
        <location filename="../qml/stages/Storage.qml" line="92"/>
        <source>Storage R/W</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-vulkan-missing">
        <location filename="../qml/stages/Vulkan.qml" line="46"/>
        <location filename="../qml/stages/Vulkan.qml" line="58"/>
        <location filename="../qml/stages/Vulkan.qml" line="88"/>
        <source>no Vulkan here</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="id-vulkan-warming">
        <location filename="../qml/stages/Vulkan.qml" line="61"/>
        <location filename="../qml/stages/Vulkan.qml" line="90"/>
        <source>warming up</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
