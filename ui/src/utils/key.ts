// 常见键码与文本的映射表
const KeyCodeMap = {
  8: 'Backspace',
  9: 'Tab',
  13: 'Enter',
  16: 'Shift',
  17: 'Ctrl',
  18: 'Alt',
  19: 'Pause/Break',
  20: 'Caps Lock',
  27: 'Escape',
  32: 'Space',
  33: 'Page Up',
  34: 'Page Down',
  35: 'End',
  36: 'Home',
  37: '←',
  38: '↑',
  39: '→',
  40: '↓',
  45: 'Insert',
  46: 'Delete',
  112: 'F1',
  113: 'F2',
  114: 'F3',
  115: 'F4',
  116: 'F5',
  117: 'F6',
  118: 'F7',
  119: 'F8',
  120: 'F9',
  121: 'F10',
  122: 'F11',
  123: 'F12',
  144: 'Num Lock',
  145: 'Scroll Lock',
  160: 'Shift',
  161: 'Shift',
  162: 'Ctrl',
  163: 'Ctrl',
  164: 'Alt',
  165: 'Alt',
  186: ';',
  187: '=',
  188: ',',
  189: '-',
  190: '.',
  191: '/',
  192: '`',
  219: '[',
  220: '\\',
  221: ']',
  222: "'",
} as { [key: number]: string }

type ModifierKey = 16 | 17 | 18
export type KeyCombo = [ModifierKey[], number]

/**
 * 将键盘事件中的 keyCode 转换为可读文本
 * @param keyCode 键盘事件的键码
 * @returns 对应的键名称或 'Unknown'
 */
export const keyCodeToText = (keyCode: number): string => {
  // 处理字母 A-Z
  if (keyCode >= 65 && keyCode <= 90) {
    return String.fromCharCode(keyCode)
  }

  // 处理数字 0-9（主键盘）
  if (keyCode >= 48 && keyCode <= 57) {
    return String.fromCharCode(keyCode)
  }

  // 处理小键盘数字 0-9
  if (keyCode >= 96 && keyCode <= 105) {
    return `Num ${keyCode - 96}`
  }

  // 处理特殊键
  return KeyCodeMap[keyCode] || 'Unknown'
}

const modifierMap: { [key: number]: ModifierKey } = {
  16: 16,
  17: 17,
  18: 18,
  160: 16,
  161: 16,
  162: 17,
  163: 17,
  164: 18,
  165: 18,
}

const isModifierKey = (key: number) => {
  return [16, 17, 18, 160, 161, 162, 163, 164, 165].includes(key)
}

export const convertToKeyCombo = (keys: number[]): KeyCombo => {
  const modifiers: ModifierKey[] = []

  for (const key of keys) {
    if (isModifierKey(key)) {
      if (modifiers.indexOf(modifierMap[key]) === -1) {
        modifiers.push(modifierMap[key])
      }
    } else {
      return [modifiers, key]
    }
  }

  return [modifiers, 0]
}
export const keyCombosIsEqual = (comboA: KeyCombo, comboB: KeyCombo): boolean => {
  // 解构两个组合键的修饰键和主键
  const [modifiersA, mainKeyA] = comboA
  const [modifiersB, mainKeyB] = comboB

  // 1. 比较主键
  if (mainKeyA !== mainKeyB) {
    return false
  }

  // 2. 比较修饰键集合
  if (modifiersA.length !== modifiersB.length) {
    return false
  }

  // 3. 检查所有修饰键是否相同
  for (const key of modifiersA) {
    if (modifiersB.indexOf(key) === -1) {
      return false
    }
  }

  return true
}

/**
 * 将 KeyCombo 转换为可读文本表示
 * @param combo 按键组合 [修饰键集合, 主键]
 * @returns 可读的文本表示
 */
export const keyComboToText = (combo: KeyCombo): string => {
  const [modifiers, mainKey] = combo

  // 1. 处理修饰键（按固定顺序：Ctrl, Shift, Alt）
  const modifierTexts: string[] = []

  // 按特定顺序检查修饰键
  if (modifiers.indexOf(17) !== -1) {
    modifierTexts.push('Ctrl')
  }
  if (modifiers.indexOf(16) !== -1) {
    modifierTexts.push('Shift')
  }
  if (modifiers.indexOf(18) !== -1) {
    modifierTexts.push('Alt')
  }

  // 2. 处理主键
  const mainKeyText = mainKey === 0 ? '' : keyCodeToText(mainKey)

  // 3. 组合结果
  if (modifierTexts.length > 0) {
    return `${modifierTexts.join('+')}+${mainKeyText}`
  }

  return mainKeyText
}
