import { defineStore } from 'pinia'
import { reactive } from 'vue'
interface Time {
  time: number
}
interface Mouse extends Time {
  x: number
  y: number
}
interface MouseLeftButtonDown extends Mouse {
  type: 'leftButtonDown'
}
interface MouseLeftButtonUp extends Mouse {
  type: 'leftButtonUp'
}
interface MouseRightButtonDown extends Mouse {
  type: 'rightButtonDown'
}
interface MouseRightButtonUp extends Mouse {
  type: 'rightButtonUp'
}
interface MouseMove extends Mouse {
  type: 'move'
}
export const MouseTypeMap = {
  0: 'leftButtonDown',
  1: 'leftButtonUp',
  2: 'rightButtonDown',
  3: 'rightButtonUp',
  4: 'move',
} as {
  [key: number]: 'leftButtonDown' | 'leftButtonUp' | 'rightButtonDown' | 'rightButtonUp' | 'move'
}
type MouseEvent =
  | MouseLeftButtonDown
  | MouseLeftButtonUp
  | MouseRightButtonDown
  | MouseRightButtonUp
  | MouseMove

interface Keyboard extends Time {
  code: number
}
interface KeyboardKeyDown extends Keyboard {
  type: 'keyDown'
}
interface KeyboardKeyUp extends Keyboard {
  type: 'keyUp'
}
export const KeyboardTypeMap = {
  0: 'keyDown',
  1: 'keyUp',
} as {
  [key: number]: 'keyDown' | 'keyUp'
}
type KeyboardEvent = KeyboardKeyDown | KeyboardKeyUp

// SortedEvents 代理类
class SortedEvents<T extends Time> {
  private _events: T[]
  //private _version: number // 增加版本号用于跟踪变化

  constructor() {
    this._events = []
    //this._version = 0
  }

  // 添加事件并保持排序
  add(event: T): void {
    const index = this._events.findIndex((e) => e.time > event.time)
    if (index === -1) {
      this._events.push(event)
    } else {
      this._events.splice(index, 0, event)
    }
    //this._version++
  }

  // 移除事件
  remove(event: T): void {
    const index = this._events.indexOf(event)
    if (index !== -1) {
      this._events.splice(index, 1)
      //this._version++
    }
  }

  // 获取有序数组（只读）
  get list(): Readonly<T[]> {
    return [...this._events] // 返回数组副本
  }

  // // 获取当前版本号
  // get version(): number {
  //   return this._version
  // }

  // 遍历方法
  forEach(callback: (event: T) => void): void {
    this._events.forEach(callback)
  }

  // 映射方法
  map<U>(callback: (event: T) => U): U[] {
    return this._events.map(callback)
  }

  // 过滤方法
  filter(callback: (event: T) => boolean): T[] {
    return this._events.filter(callback)
  }

  // 清空事件
  clear(): void {
    this._events = []
    // this._version = 0
  }
}

export const useRecordStore = defineStore('record', () => {
  // 为 MouseEvent 和 KeyboardEvent 创建独立实例
  const mouseEvents = reactive(new SortedEvents<MouseEvent>())
  const keyboardEvents = reactive(new SortedEvents<KeyboardEvent>())
  const getPressedKeys = () => {
    // 存储每个按键的最后状态和最后一次按下的时间
    const keyState = new Map<number, { pressed: boolean; lastDownTime: number }>()

    // 正序遍历所有事件，确保状态按时间顺序更新
    for (const event of keyboardEvents.list) {
      // 获取当前按键状态（如果不存在则创建默认状态）
      const state = keyState.get(event.code) || {
        pressed: false,
        lastDownTime: -1,
      }

      // 根据事件类型更新状态
      if (event.type === 'keyDown') {
        state.pressed = true
        state.lastDownTime = event.time // 记录最后一次按下的时间
      } else if (event.type === 'keyUp') {
        state.pressed = false
      }

      // 更新状态记录
      keyState.set(event.code, state)
    }

    // 收集当前按下的按键及其最后一次按下的时间
    const pressedWithTime: { code: number; time: number }[] = []
    keyState.forEach((state, code) => {
      if (state.pressed && state.lastDownTime >= 0) {
        pressedWithTime.push({ code, time: state.lastDownTime })
      }
    })

    // 按最后一次按下的时间排序（从早到晚）
    pressedWithTime.sort((a, b) => a.time - b.time)

    // 提取按键代码数组
    return pressedWithTime.map((item) => item.code)
  }
  return { mouseEvents, keyboardEvents, getPressedKeys }
})
