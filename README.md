

### fix的 block版本（條件變數）  queue版本 一個生產者一個消費者 
```
g++ -std=c++11 -pthread -o queue_fix_block queue_fix_block.cpp
```
```
./queue_fix_block
```


### fix的 block版本（queue_fix_mesure） 量測響應時間
```
g++ -std=c++11 -pthread -o queue_fix_mesure queue_fix_mesure.cpp
```
```
./queue_fix_mesure
```


這段程程式碼的解釋