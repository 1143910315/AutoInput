import { type RemovableRef, useStorage } from '@vueuse/core'

import { convertToKeyCombo, type KeyCombo, keyCombosIsEqual, keyComboToText } from '../utils/key'

// 热键配置类型定义
interface HotkeyConfig {
  id: string
  combo: KeyCombo
}

// 默认热键配置
const defaultRecordHotkey: HotkeyConfig = {
  id: 'record',
  combo: [[], 112],
}
class Hotkey<T extends RemovableRef<HotkeyConfig>, U extends HotkeyConfig> {
  private _hotkey: T
  private _defaultHotkey: U

  constructor(hotkey: T, defaultHotkey: U) {
    this._hotkey = hotkey
    this._defaultHotkey = defaultHotkey
  }

  reset() {
    this._hotkey.value = this._defaultHotkey
  }

  trySet(keyList: number[]) {
    const keyCombo = convertToKeyCombo(keyList)
    if (keyCombo[1] !== 0) {
      this.set(keyCombo)
    }
    return false
  }

  set(keyCombo: KeyCombo) {
    this._hotkey.value.combo = keyCombo
  }

  equals(keyCombo: KeyCombo) {
    return keyCombosIsEqual(this._hotkey.value.combo, keyCombo)
  }

  toString() {
    return keyComboToText(this._hotkey.value.combo)
  }
}

export const recordHotkey = new Hotkey(
  useStorage<HotkeyConfig>('hotkey-record-config', defaultRecordHotkey),
  defaultRecordHotkey,
)
