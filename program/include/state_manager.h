#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

/**
 * 状態マップエントリー
 */
struct StateMapEntry
{
    int sensor_state; // センサー状態（6ビット値）
    int group_id;     // グループ ID（0～4）
};

/**
 * 状態管理クラス
 * センサー状態をグループに分類し、状態遷移を追跡
 */
class StateManager
{
private:
    StateMapEntry *state_map; // 状態マップテーブル
    int map_size;             // マップサイズ

    int current_state; // 現在の確定状態（グループ ID）
    int state_count;   // 同じ状態が続いたフレーム数

    int prev_state;       // 前回の確定状態
    int prev_state_count; // 前回の状態継続フレーム数

public:
    /**
     * コンストラクタ
     */
    StateManager() : state_map(nullptr), map_size(0), current_state(-1),
                     state_count(0), prev_state(-1), prev_state_count(0) {}

    /**
     * デストラクタ
     */
    ~StateManager()
    {
        if (state_map != nullptr)
        {
            delete[] state_map;
            state_map = nullptr;
        }
    }

    // コピーコンストラクタとコピー代入演算子を削除（ダブルデリート防止）
    StateManager(const StateManager &) = delete;
    StateManager &operator=(const StateManager &) = delete;

    /**
     * 状態マップを設定
     * @param map 状態マップ配列
     * @param size マップサイズ
     */
    void setStateMap(StateMapEntry *map, int size)
    {
        // 入力チェック
        if (map == nullptr || size <= 0)
            return;

        if (state_map != nullptr)
        {
            delete[] state_map;
            state_map = nullptr;
        }

        state_map = new StateMapEntry[size];
        map_size = size;

        for (int i = 0; i < size; i++)
        {
            state_map[i] = map[i];
        }
    }

    /**
     * センサー状態をグループ ID にマッピング
     * @param sensor_state センサー状態（6ビット値）
     * @return グループ ID（未登録の場合は -1）
     */
    int mapState(int sensor_state)
    {
        for (int i = 0; i < map_size; i++)
        {
            if (state_map[i].sensor_state == sensor_state)
            {
                return state_map[i].group_id;
            }
        }
        return 0; // 未登録状態
    }

    /**
     * 状態を更新し、遷移を検出
     * @param sensor_state 現在のセンサー状態
     * @return 状態遷移が確定した場合は true
     */
    bool updateState(int sensor_state)
    {
        int mapped = mapState(sensor_state);

        // 未登録状態の場合は前の状態を維持
        if (mapped == -1)
        {
            mapped = current_state;
        }

        // 状態が変わったかチェック
        bool state_changed = (mapped != current_state);

        // 前回の状態を記録
        if (state_changed)
        {
            prev_state = current_state;
            prev_state_count = state_count;
            state_count = 1;
        }
        else
        {
            // 同じ状態が続いている場合、カウントを増やす
            if (state_count < 30000)
            {
                state_count++;
            }
        }

        // 現在の状態を即座に更新
        current_state = mapped;

        return state_changed; // 状態遷移が発生した場合は true
    }

    /**
     * 現在の確定状態を取得
     * @return グループ ID
     */
    int getCurrentState()
    {
        return current_state;
    }

    /**
     * 前回の確定状態を取得
     * @return グループ ID
     */
    int getPrevState()
    {
        return prev_state;
    }

    /**
     * 現在の状態継続フレーム数を取得
     * @return フレーム数
     */
    int getCurrentStateCount()
    {
        return state_count;
    }

    /**
     * 前回の状態継続フレーム数を取得
     * @return フレーム数
     */
    int getPrevStateCount()
    {
        return prev_state_count;
    }

    /**
     * 状態をリセット
     */
    void reset()
    {
        current_state = -1;
        state_count = 0;
        prev_state = -1;
        prev_state_count = 0;
    }
};

#endif // STATE_MANAGER_H
