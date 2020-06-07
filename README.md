# Simple FBX Viewer

# 特徴
* JSON形式のシーンファイルのロード
* FBXSDKを使ったFBXファイルのロード
* OpenGLレンダリング
* スケルタルアニメーション
* 簡易的なアニメーションステートマシン

# 使用する外部ライブラリ
* FBXSDK 2019
* OpenGL 3.3
* SDL2
* stb

# 動作保証
* MacOS Catalina version 10.15.3 + Xcode version 11.3.1

# 使用方法
1. アセットディレクトリを以下の階層構造で用意する。

```
//hierarchical structure of asset directory
- asset_directory (any name)
    - scene.json (json file)
    - mesh (directory)
        - mesh1.fbx
        - mesh2.fbx
        - ...
        - ...
    - texture (directory)
        - texture1.png
        - texture2.png
        - ...
        - ...
    - animation (directory)
        - animation1.fbx
        - animation2.fbx
        - ...
        - ...
```
2. scene.jsonという名前のシーンファイルを用意する。書き方は「シーンファイルの書き方」を参照。
3. アプリケーションを実行するとコマンドライン上でアセットディレクトリの絶対パスを入力するよう指示されるので
ドラッグ&ドロップ等で入力する。
4. Viewerが起動する。カメラは円柱座標系に従って動く。カメラの視線は上下の方向キーで調整可能。


# シーンファイルの書き方
* シーンファイルは以下のような構造を持っていなければならない。
```
//scene.json
{
    "mesh":{
        //スケルタルアニメーションを希望する場合はtrue、それ以外はfalse
        "is_skeletal":true or false,
        
        //meshの表面の色として何を出力するか
        //"texture"を選ぶと、FbxMaterialのdiffuse textureに指定されているtextureが使用される
        "color":"texture" or "uv" or "normal",
        
        //mesh directoryに存在するFBXファイル名を入力
        "filename":"mesh file name"
    },
    
    //この項目は"is_skeletal"がtrueの時のみ書けば良い
    "animation_controller":{
        //アニメーションステートマシンのエントリーステートを指定する
        "entry_state_id":"state id",
        
        //ステートを一つ以上定義する
        "states":[
            {
                //ステートのidを定義
                "id":"state id",
                
                //animation directoryに存在するFBXファイル名を入力
                "animation":"animation file name"
                
                //自動遷移の定義、定義しない時は空オブジェクトにする
                //自動遷移はユーザーがトリガーを引かなくとも自動で発生する遷移
                //自動遷移は遷移元ステートのアニメーションの終端を含むように遷移する
                "auto_transition":{
                    //現在のステートにおけるアニメーションの長さのうち何割を遷移に使うかを示す正規化された遷移時間
                    "duration":0~1の数値,
                    
                    //遷移先ステートにおけるアニメーションの開始時刻を示す正規化された時間
                    "offset":0~1の数値,
                    
                    //遷移先ステートのidを入力
                    "dst_state_id":"state id"
                },
                
                //ユーザー遷移の定義、定義しない時は空配列にする
                //ユーザー遷移はユーザーのキーボード入力によって発動する
                "user_transitions":[
                    {
                        //このトリガーがユーザーから入力されると、遷移が始まる。
                        "trigger":"keyboard key",
                        
                        //トリガーが入力された後、即座に遷移が始まるようにしたいならtrue
                        //遷移元ステートのアニメーションの終端を含むように遷移させたいならfalse
                        "is_immediate":true or false,
                        
                        //現在のステートにおけるアニメーションの長さのうち何割を遷移に使うかを示す正規化された遷移時間
                        "duration":0~1の数値,
                        
                        //遷移先ステートにおけるアニメーションの開始時刻を示す正規化された時間
                        "offset":0~1の数値,
                        
                        //遷移先ステートのidを入力
                        "dst_state_id":"state id"
                    },
                    {
                        //二つ目の遷移の定義
                    },
                    {
                        //三つ目の遷移の定義
                    }
                ]
                
                
            },
            {
                //二つ目のステートの定義
            },
            {
                //三つ目のステートの定義
            }
        ]
    }
}
```


