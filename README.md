Nanoshowat
==========

Summary
-------
クリッカーのコントロールソフトウェア "Clicker Nano" では, 通常の多肢選択投票時には, リアルタイムで各端末の受信状況を一覧できる. 一方, 出席確認の際には, ポップアップウィンドウ内でスクロールしない限り, リアルタイムで各端末の受信状況を一覧することはできない. 出席確認終了後には. CSVで保存された結果を, 表計算ソフトウァアなどで閲覧することにより, 受信状況を確認することができる.
nanoshowat は, このCSVファイルを解析し, 学籍番号リスト GakusekiList.txt と照合して, 受信状況を, スクロール無しに要約してコマンドプロンプト内で表示するプログラムである.

Summary
-------
Displays attendance results of Clicker Nano. http://finewz.com/type_clicker.html
This program reads AttendanceResult.csv and GakusekiList.txt and pretty-print the result. It is not a real-time report like the overlay for normal voting. Only terminated attendance checks can be summarized.

Example of Output
-----------------
    c:\Users\username\Documents>nanoshowat -f samples\2013112702_AttendanceList.csv
    AttendanceList:samples\2013112702_AttendanceList.csv
    gakusekiList:gakusekiList.txt
    Range: All today's attendance check
    Latest: .
    Gakuseki verified: *
    
                103*  104*  105*. 106*  107*.             110*
    111*.       113*  114*. 115*        117*  118*  119*  120*.
          122*  123*.                         128*  129*. 130*
    131*  132*  133*. 134*. 135*  136*.             139*.
    141*. 142*  143*  144*  145*        147*  148*  149*. 150.
          152*. 153*.       155*  156*  157*  158*  159*. 160*
    161*  162*        164*. 165*  166*  167*.       169*
          172*. 173*. 174*  175*  176*  177*. 178*  179*. 180*.
    181*  182*  183*  184*  185*  186*.             189*  190*
    191*  192*  193*  194*  195*. 196*. 197*  198*. 199*. 200*
    
Tested Plafform
---------------

* Windows Vista
* Visual C++ 2008 with Configuration Property > General > Character set > multibyte string enabled
    * 構成プロパティ > 全般 > 文字セット > マルチバイト文字列 にチェック
* Clicker Nano Professional Edition for 2003, Build 1.0.1.120111 Model-T

Options
-------
The one specified later in the command line has priority.

* -h show this help
* -v verbose mode
* -d <AttendanceDataDirectoryName>
* -f <AttendanceCSVFileName>
* -g <GakusekiListFileName>
* -m use GakusekiList.txt in my home directory like Nano
* -c use GakusekiList.txt in the current directory (default)
* -n do not use gakusekiList.txt but guess from Nano's output
* -i ignore gakuseki list. this overrides -g, -m, -c and -n.
* -s analyze the last attendance check result only
