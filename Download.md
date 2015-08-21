# ダウンロード #

### 安定版 (1.8.x系) (Stable version) ###

#### 1.8.4 (2013/06/13) ####
  * [インストーラー版](https://code.google.com/p/binaryeditorbz/downloads/detail?name=BzEditor-1.8.4.exe&can=2&q=#makechanges)
  * [Zip版](https://code.google.com/p/binaryeditorbz/downloads/detail?name=Bz184.zip&can=2&q=#makechanges)


### 最新テスト版 (1.9.x系) (Unstable version) ###

#### 1.9.6 (2014/08/10) ####
  * [インストーラー版](https://drive.google.com/file/d/0B7h5NcRw8uVCU0UtUFNxNGk5RkU/edit?usp=sharing)
  * [Zip版](https://drive.google.com/file/d/0B7h5NcRw8uVCRlBCcFlNQldHbUE/edit?usp=sharing)

  * 1.9.6 (2014/08/10)
  * (修正) [Issue 37](https://code.google.com/p/binaryeditorbz/issues/detail?id=37) ショートカットキーが効かないバグ (Spetial Thanks! gnanashi0)
  * (修正) [Issue 37](https://code.google.com/p/binaryeditorbz/issues/detail?id=37) ダンプリストの保存時に入力したファイル名が反映されないバグ (Special Thanks! gnanashi0)
  * (修正) [Issue 37](https://code.google.com/p/binaryeditorbz/issues/detail?id=37) 検索窓のEnterキーが効かないなどのバグ (Special Thanks! gnanashi0)
  * (追加) [Issue 38](https://code.google.com/p/binaryeditorbz/issues/detail?id=38) ショートカットキー追加 (Special Thanks! gnanashi0)
  * (修正) [Issue 39](https://code.google.com/p/binaryeditorbz/issues/detail?id=39) 分割表示の際カーソルを新しいビューへ移動 & 分割表示解除時、カーソルが無いビューを削除するよう変更 (Special Thanks! gnanashi0)
  * (修正) [Issue 39](https://code.google.com/p/binaryeditorbz/issues/detail?id=39) 分割表示解除時、ファイルの変更が保存されないバグ (Special Thanks! gnanashi0)
  * (追加) [Issue 38](https://code.google.com/p/binaryeditorbz/issues/detail?id=38) ショートカットキー追加 2 (Special Thanks! gnanashi0)
  * (修正) [Issue 41](https://code.google.com/p/binaryeditorbz/issues/detail?id=41) 日本語入力が出来ないバグ (Special Thanks! GooChan608)
  * (修正) [Issue 41](https://code.google.com/p/binaryeditorbz/issues/detail?id=41) ステータスバーの"上書/書禁"の切り替えができないバグ (Special Thanks! GooChan608)
  * (修正) "指定アドレスへ", "値の入力"実行時のキャレット移動処理追加
  * (修正) サブビュー無し＆左右分割の場合、左のビューのサイズが小さすぎるバグ
  * (修正) 新アドレス表示 & 削除、カットが動かないバグ修正
  * (修正) 常に16桁で表示にもカラー表示追加
  * (修正) [Issue 40](https://code.google.com/p/binaryeditorbz/issues/detail?id=40): 構造体表示が名前しか表示されないバグ
  * (修正) [Issue 43](https://code.google.com/p/binaryeditorbz/issues/detail?id=43): 読み込み時リードオンリーが働いてない
  * (追加) [Issue 42](https://code.google.com/p/binaryeditorbz/issues/detail?id=42): 右クリックメニューがほしい

  * 1.9.5 (2014/07/06)
  * (修正) [Issue 36](https://code.google.com/p/binaryeditorbz/issues/detail?id=36)  引数にダブルクォートされたファイル名を与えても開けない (Special Thanks: gnanas)
  * (修正) [Issue 36](https://code.google.com/p/binaryeditorbz/issues/detail?id=36)  ウィンドウサイズが自動調整されないバグ (Special Thanks: gnanas)
  * (修正) ツールバー、ステータスバー、グリッド表示のメニューを削除 (Special Thanks: gnanas)
  * (追加) [SuperFileCon](SuperFileCon.md) 高速保存の追加
  * (追加) 64ビット版を追加
  * (修正) 切り取り、削除、コピーが0xFFFFffff超えると失敗するバグ
  * (修正) 設定パネルを改修
  * (修正) ステータスバーが正しく表示されないバグ
  * (追加) "設定のリセット"を追加

  * 1.9.4 (2014/06/01)
  * (修正) コマンドライン引数でファイル名を与えても開かないバグ
  * (修正) ウィンドウと閉じる際にBZDoc2を解放し忘れ
  * (修正) 半バイト分入力時に表示が更新されないバグ / 同じファイルを分割ビューで表示しているとき更新されないバグ

  * 1.9.3 (2014/05/31)
  * (修正) [Issue 27](https://code.google.com/p/binaryeditorbz/issues/detail?id=27) スクロールバー操作で落ちます (Special Thanks: gnanas)
  * (修正) [Issue 30](https://code.google.com/p/binaryeditorbz/issues/detail?id=30) UTF8の日本語表示がおかしい (Special Thanks! takamitsu)
  * (追加) [Issue 31](https://code.google.com/p/binaryeditorbz/issues/detail?id=31) BMPビューの旧BZカラーパレットサポート (Special Thanks! g1e2n04)
  * (追加) 4GB超ファイルに対応
  * (追加) 完全WTL化（脱MFC）
  * (修正) UTF16が文字化けする
  * (修正) 新規作成 + SaveAsした場合、GetFilePath()がNULLを返す（パスが更新されない）バグ
  * (修正) SuperFileCon: 一回で50MB以上の削除をすると、保存時にアクセス違反でクラッシュするバグ
  * (修正) キーを押したまま入力中に表示が更新されない / HDDがガリガリする

### 更新履歴 (Changelog) ###
  * [ヘルプ](http://devil-tamachan.github.io/BZDoc/#changelog.changelog)

### その他バージョン ###

  * [全バージョンアーカイブ (Google Drive)](https://drive.google.com/folderview?id=0B7h5NcRw8uVCWUQ3WlNIVkZXQ0E&usp=drive_web)
  * [1.9.2以前までのダウンロードページ](https://code.google.com/p/binaryeditorbz/downloads/list)