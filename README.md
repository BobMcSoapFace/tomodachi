# tomodachi

![zine poster of the panda project](/photos/zine_sized.jpg)

Tomodachi is a project to give you a heavily dependent and lazy panda to take care of, made because I need hardware and CAD design experience plus coding in firmware and also making panda themed gadgets is one of the coolest activities of all time

What the panda can do:
* Eat the food you give them (until they don't feel like it)
* Be head patted
* Perish

Features hand-drawn animations (more animations will come out once project is approed since I need the RTC to test more)


# materials

Name  | Quantity | Purchase link
------------- | ------------- | --------
ESP32C3  | 1 | [XIAO ESP32C3 DIP link](https://www.digikey.com/en/products/detail/seeed-technology-co-ltd/113991054/16652880)
DS3231 RTC | 1 | [DS3231 RTC link](https://www.ebay.com/itm/297741188722?chn=ps&google_free_listing_action=view_item)
Coin battery (for RTC if not included) | 1 | [Coin battery link](https://www.amazon.com/Murata-CR2032-Battery-Lithium-Coin/dp/B0B86LLWL8/ref=sr_1_2?crid=5N97OZSN6Z8H&dib=eyJ2IjoiMSJ9.F8PfWrmyizPbCG1gGso4Sn2BNfQ0eB1IYDHcMVNKymwzoMyjxj2rfjve1XwerShSBkc4a-dxVw_WuKlAkhaegxfFCFuhnR8R5kZjfYY0XphpTUnMTNZGNMzcZ4h64JS-bIkLakEKzOY6vGOThsG29ine8_IvoT9ieV70WUatbNnUaVEURMSSpseiIEaj0lkrKBS16BPGaSFWcJ-ryjQvp6fFocNM3I8KLXsGqz4SF25_1Fuxvltx6ggZdItXhQ_UIjJVnESPV-SKaemzGp7qIv84hWzi2OfmfkO0jIdj91Y.-Nvivpyu0MaXKH9qYVh21XHtwmFQd-MbeiVkGIs3xcA&dib_tag=se&keywords=cr2032+batteries&qid=1778205699&s=hpc&sprefix=CR2032+b%2Chpc%2C135&sr=1-2)
0.96in SSD1306 (128x64) | 1 | [SSD1306 (128x64)](https://www.aliexpress.us/item/3256807708805773.html?src=google&gatewayAdapt=glo2usa)
TP4056 | 1 | [TP4056 link](https://www.aliexpress.us/item/3256810410652428.html?src=google&gatewayAdapt=glo2usa)
Shottky diodes | 2 | this was the cheapest & most appropriate I could find -> [Schottky diodes link](https://www.amazon.com/VKLSVAN-15SQ045-Schottky-Diodes-Blocking/dp/B0DQGSDM6K/ref=sr_1_3?dib=eyJ2IjoiMSJ9.PRD4gvHdvDkC7SWHPaThgMwinM6Hy03ManybLbNwocMLrMJ9t_UkzCJYTdK0w7Lp_8ZR3MuXPNbKxz-Bt2SL9jdP8HabjVXBSTa6D2bg_bAsov6v16EMSgxGUxbWkNcjPib12Atslzx8aFsGQUOQaBg7FGf3G99chlaEFD5ZFAB9ur6NqzZEgtfETEsCHgCuqURKETHtwKPl8mpUCqiVJLfFYUm0_RdZWl7DTsZKh9o.iTZxRkUnYxQ1WVMXztBbVDkU00ySDiqvC2g8iIOHOFA&dib_tag=se&keywords=schottky+diode&qid=1778206254&refinements=p_85%3A2470955011&rnid=2470954011&rps=1&sr=8-3)
KY-040 Rotary Encoder | 1 | [KY-040 Rotary encoder link](https://www.aliexpress.us/item/3256810332864684.html?src=google&gatewayAdapt=glo2usa)
SS12F44 Switch | 1 | [SS12F44 Switch link](https://www.aliexpress.us/item/3256804709322450.html?src=google&gatewayAdapt=glo2usa)
Battery 18650 3.7V | 1 | [18650 battery link](https://www.digikey.com/en/products/detail/tinycircuits/ASR00050/9808766?gclsrc=aw.ds&gad_source=4&gad_campaignid=20243136172&gbraid=0AAAAADrbLlidfmJ5VH8HTIKUcIAKufJXb&gclid=CjwKCAjwtvvPBhBuEiwAPMijr8Klf8WiFOSPZoRulJW1RZkaHeSNISgGmuLmGddLyfp-fYC6PziE-xoCvMIQAvD_BwE)
Wires | |

(the material links were used assuming the fallout team has amazon prime for free shipping)

# design
## PCB Schematic
![PCB schematic](/photos/schematic.png)
^note that the part labeled E1 is because Kicad was mad I have the SS12F44 switch connected by wires instead of traces, please ignore
## PCB Footprint
![PCB footprint](/photos/pcb.png)
## Case
![Inside the 3D modeled case](/photos/case_inner.png)
![Outer 3D modeled case](/photos/case.png)
# assembly
steps:
1. 3d print the case, case cover and panda ears
   - PLA is going to be used in this project but recommended to use a more stronger & less brittle filament for snap fits
2. Flash the firmware by using Arduino IDE:
   1. Download the firmware folder and place the firmware folder inside a dedicated folder
   2. Unzip the libraries.zip and place "libraries" folder in the dedicated folder (named exactly that)
   3. Go to Arduino IDE File > Preferences and set the library folder to libraries
   4. Connect the XIAO ESP32C3 DIP to the computer with a USB-C cable, select the port that pops up and set the board as a "XIAO_ESP32C3"
   5. Go to "tomodachi_sketch.ino" and compile and upload
3. Solder the components to the PCB
   - Use header pins to connect the throughholes on both the pcb and components for easier soldering (though tinning the holes will work too)
   - Make sure to insert the coin battery into the RTC after soldering (heat damage might hurt the battery)
4. Solder a wire from the bottom left corner pad marked "CONNECT TO SWITCH" (<ins>not</ins> the throughholes connected to the D2 schottky diode) to one end of the SS12F44 switch's three pins, and then solder a wire from the Out+ of the TP4056 component (also marked with "CONNECT TO SWITCH") to the middle pin on the SS12F44 switch's three pins
   - The SS12F44 connects the middle pin with one or the other of the three pins depending on where it is switched
   - Use a short length wire when soldering the two places, and wrap each connection in electrical tape to ensure no short circuits happen
   - For both of the mentioned throughholes on the PCB probably don't use header pins as it will be easier to solder a wire without them
5. Place the battery in the case first, then solder the wire from the battery's + end to the TP4056's 5V hole, and the battery's - end to the TP4056's GND hole (both located on the edge of the PCB)
6. Assemble case with cover and ears