# Key Value Storage Report
[2020_OS_Fall_HW3: Key-Value Stroages](/@dsclab/rJDIP8YYv)

> Author INFO：  
> F74076019  
> 黃宇衡  
> 資工111

## Develope Environment (Virtual Machine)
* OS : Ubuntu 18.04.2 LTS
* CPU : Intel(R) Core(TM) i7-9700 CPU @ 3.00GHz * 4
* System Memory : 4G
* HardDisk capacity：80G
* Programming Landuage : C++ 
* Compiler : g++ (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
* VMware : Oracle Virtual Box  6.1.14 r140239 (Qt5.6.2)

## How to run my code
```shell=
# compile
g++ key-value.cpp -o key-value -O3

# execution
./key-value [input file path]

```
ex:
```shell=
g++ key-value.cpp -o key-value -O3
./key-value ./hw3example.input
```

## Program Developement & Runtime result
### Development method：
本次資料庫開發運到C++的STL容器map和queue來實作，map負責儲存key-value的對應關係，且map底層是用紅黑樹的資料結構實作，在插入、尋找、刪除一個key-value可以有很高的效率。queue的部分則是用來管理有多少個檔案被從檔案系統裡讀入到記憶體並儲存成map型態，此部分在後續的分析報告裡會詳細說明，這邊僅討論資料的I/O以及處理邏輯。  
當程式開始執行時，會讀入指令檔，依照指令是PUT，GET，SCAN的不同而呼叫不同函數來處理：  
* PUT()  
    當呼叫PUT()時程式會呼叫getmap()這個函數，這個函數會計算這個Key應該存入的map的index，並去記憶體或檔案系統中尋找對應的map回傳，接著插入新的或修改該map中的key-value
* GET()  
    當呼叫GET()函數時時程式也會呼叫getmap()這個函數，一樣計算這個Key應該存入的map的index，並去記憶體或檔案系統中尋找對應的map回傳，但接著調用C++ map內建的find()函數，該函數如果有找到key會回傳該key的iterator位置，若找不到則回傳該map的end iterator位置，依據這樣的回傳，決定回傳找的value或是"EMPTY"
* SCAN()  
    僅用for迴圈反覆執行多次的GET()
    
### Runtime Result：
以範例測資為例
![](https://i.imgur.com/pThEWar.png)

## Analysis Report 
### How to manage my database
接續前面的開發方法，本段落會說明如何設計並管理我的資料庫以及如何獲取最佳的存取的效率。  

我將一個大的資料庫拆分成複數個小的資料庫儲存，這些小的資料庫在記憶體裡是複數個map，而這複數個map則是透過一個queue來管理。當小的資料庫被存到檔案系統時，則是以字串的的方式將key和value的關係以```[key]:[value]```的格式儲存。  

先說明如何用queue管理複數個map，先前講到PUT()，GET()，SCAN()均需要呼叫getmap()函數來得到正確的map，但這個正確的map()並不一定隨時都在記憶體中，有可能會因為長久沒被存取而為了釋放空間被下放到檔案系統。   
1. getmap()會得到一個要查找map的index，接著先在queue裡還有的map找找看，如果存在就回傳該map並將map在queue的位置置換到最尾端，達到queue中最尾端的map一定是most recently used的效果。  
2. 如果不存在於queue中則去檔案系統裡找[index].tmp這個檔名是否存在，如果有則呼叫loadmap()將檔案系統的字串檔讀入，回復成map的格式後push_back()到queue的最尾端，在push_back()前須先檢查queue是否還有空位，如果queue已滿，則呼叫writemap()函數並pop_front()移除queue開頭最久未被使用的map，並將其寫回檔案系統  
3. 如果queue裡無法找到指定index的map且[index].tmp也不存在在檔案系統中，則代表這個map是一個新的map，因此創造一個新的map後在-1的key裡儲存該map的index並push_back()到queue的最尾端，同第二點在push_back()前須先檢查queue是否還有空位

從上述的步驟得知，map必須含有存有index資訊，否則無法快速查找指定的map，index的用途、如何計算以及儲存方式將在這個段落說明  
* 為了分割大資料庫和快速查找map，我引入index以及file header的觀念，首先決定一個map的容量，也就是key-value的筆數，而index就是給定一個key除上一個map的容量，這樣的效果可以讓每個map儲存一樣的容量，也可以快速得知這個map中key的range，而這樣的index就會被儲存在map的開頭，也就是key為-1位置當作是map的header。
* 舉例來說，假設今天每個map的容量是100筆key-value，則```key:value``` = ```568:mytestvalue01```會被儲存在index為5的map裡，也就表示key 568被儲存在的map中，有的key-value的對應關係是```-1:5```和```568:mytestvalue01```，反之，要GET(568)只要先找map的```key:value``` = ```-1:5```的map，再從該張map中find(568)就好，不用把全部的map都找過  

按照這樣的設計邏輯，想要有最佳的存取率，必須滿足以下條件：
* queue裡的map並未被踢回檔案系統  
* key位於同一個map裡  

因此當存取連續的key且連續key的range < map的容量(不需要存取別的map)時會有最佳的存取效率，符合直觀撰寫程式的principle of locality

### Program optimization
* 上面的資料庫處理邏輯提到，每次如果map位於queue的中間，會被置換到queue的尾端，其實作方法是將位於queue中間的map begin iterator複製一次重新push_back()到queue的尾端後再erase()原本中間的map iterator，當今天連續存取同個map時會反覆將尾端的map erase()再push_back()，會增加不必要的記憶體配置時間，因此我增加新的判斷條件，若存取的map已經在最尾端，就不用重新將相同的map push_back()  
* 因為儲存資料到檔案系統是採用字串的檔案儲存，當今天只更新幾個已存在key的value時，雖然只想要更新檔案裡的幾個character，但礙於C++的開檔寫檔無法刪除檔案中已存在的位元，傳統上的做法只好將整個檔案重新寫過一遍，較沒有效率。為了解決這個問題，我選擇運用```tellp()``` ```seekp()``` ```tellg()``` ```seekg()```來操作檔案內的指標位置，並轉換想法從「刪除」舊有的key-value改成「覆寫」舊有的key-value，先將檔案內的指標移動到目標行的開頭後，再把更新後的key-value覆寫到該行上，為了避免字串資料長度不相同而導致短字串覆寫不了長字串，我將所有的key補0到19位數(long的最大長度)，讓每一行的長度都是固定19(key)+1(:)+128(value) = 148 characters，保證新字串一定能覆寫過原本的舊字串  
* 上一點的作法必須透過getline()和 string compare來比較map和.tmp中的每一行是否存在差別，但當今天map中有「新增」key-value而非「更新」已存在key的value時會因為多了新的一行，造成後續按行比較皆會得到不相同而需要覆寫該行的結果(因map的第n行應是.tmp的第n-1行)，因此當該行讀到map中的key和.tmp中的key有不同時，就會break出比較迴圈，直接將後續的map覆寫到檔案而不進行比較，節省不必要的字串比較

### OS Performance Analysis
當存取連續的9999個key時
測資前五行
```
PUT 846965132120592035 9kwBjXwwF0PvC3zofNMskkF5pAphQ9v3HD8xsk3ZTQdspNt98QgJCwH2iqZ39C3pN25yfqGTokzLNKFp1mLoDuyGlWY20eAd0djN08lY6cZHP8KVQPLdTTZiF5hqA8fM
PUT 846965132120592036 PvmUwn6RzmeEDs05mPBC1f9UZq9EOYuOt9JEwFVYcn7bj2pSwK5C16qr9ZuVGEAxbxTLxqOyheh2cwpxQGyutIMTV0OCt8Lz5fWr097aXjuRQZKoia0Aizv2Now7cZv6
PUT 846965132120592037 cZJHbEMQ4tirYYNdwq9YG3aPwvgsdoF8vbQbSpz8zSALkWPtfMOpUzvX4vAD0AeA6TTmBuMnYbzB5TrfevywwAdm6NzSUt4OxJS3FTZaHwEk82NVpB9djLAPipwSzu2L
PUT 846965132120592038 X9JNJz3BoHA97UYFzgZtz2kGDw3K5zwj5EJDhC9PpgJFrayPUlVabHpevLxRDVcgc9nwwPYGPby2f2ZBxu8GoK9KfcE2AwgFjeV5rOiAccEHb0DQJEW0LeOUGk7rfnTT
PUT 846965132120592039 PWhkyh52JF2EFcLiFJPXyfDUNlUR3CptaOSn13KTlK6CSBzXpw0l1Jwx6jKp57TidBPPXRMpfKV2Yvxcaac3xpz8gfFW7Jaz5joUJyuYUJBAdrOOIfr807ZDJsXl1JS4
PUT 846965132120592040 xrBDDiOvV1q93WyXII8OVjuGQk3fnzGCVgWfrCrnORU6ZJ2uOHDaEHFHXuue3QYUeE6aa13Lu88xz5sojeClZD1v8z9eCck5g9RSuwduYTpaYogSaTkcD0GsHAfU99th
```

![](https://i.imgur.com/wi4hXtS.png)

當存取不連續的9999個key時
測資前五行

```
PUT 1768618839572839837 SC8GmeQoeSAJa0B8TwtdVUj7pPZU6ZPsK9pTpJEdkU9XwsmdB2Pzw5z2nIWFWhLexLX3lCff1HiYgHy0SdGj8fdbuMoxnqKTklMw8DjD1aYIu83kfzb4NrK012RUHIWG
PUT 4427667222251285030 HyFZi0HTReYjBFoVvgbPshC5zoEW38pmSvViN3FbR7K1MdCxbGOTkBsA0PbdZbeeMQuac3XnEhBSDIRgLI2O57XS57C8FP2XCiAVhAHMs3J2iYjW4tSUuDC2PMZPaHW4
PUT 1586241570541266722 6DSLEZBSmtHmcY8qFXL1NFlUXJJjsgUD6JORWiclZFtkH3Jfo7g1NdAqYf9OnBoLm2bVuwawoP6rlsUuCtXcPaTpAGtPyvjd1SE4I2odUKEhGyo3DRkmXY1fOQr6ds9b
PUT 5404905251019476631 ID2tUI6NBPgkkH3t6AFIVgYkLoGmxvBtPGQ9M4ncgMYantEWIsY0vdXcK6BIk0qPkLzFuzcIRMrwyal0iyAryySQUnRnocetv9KTve3OjPBFTIRcNRphI1mCdE5pxIBl
PUT 2254804982534969088 EBcCdLacaorXlRhkW1JeUf32ruFRAdC3Xr7x5FwrW1s6mtez1HdlI12EPGQaJIh26Lq7y6RDZKn2ZIx4b0vU6LF9wMVDLLSpWdRx2sQfP26R42hsDqRO8DxT6UsSWEIg
```
![](https://i.imgur.com/8ipzghD.png)


### Reference
[Find and Modify txt File in C++](https://stackoverflow.com/questions/21212678/find-and-modify-txt-file-in-c)  
[manipulate file content pointer](https://www.decodejava.com/cpp-how-to-modify-a-file.htm)  
[map](http://www.cplusplus.com/reference/map/map/?kw=map)  