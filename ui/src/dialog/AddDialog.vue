<template>
  <el-dialog v-model="dialogVisible" title="录制新宏" width="80%" :before-close="handleClose">
    <div class="fl">
      <el-text size="large">开始/结束录制按键</el-text>
      <div class="s1"></div>
      <div class="radius" tabindex="0" ref="inputHotkey">
        <el-text type="primary" size="small">{{ recordHotkey.toString() }}</el-text>
      </div>
    </div>
    <el-table
      ref="singleTableRef"
      :data="tableData"
      highlight-current-row
      style="width: 100%"
      @current-change="handleCurrentChange"
    >
      <el-table-column property="operation" label="操作" />
      <el-table-column label="操作">
        <template #default="scope">
          <el-button size="small" @click="handleEdit(scope.$index, scope.row)">播放</el-button>
          <el-button size="small" @click="handleEdit(scope.$index, scope.row)">编辑</el-button>
          <el-button size="small" type="danger" @click="handleDelete(scope.$index, scope.row)">
            删除
          </el-button>
        </template>
      </el-table-column>
    </el-table>
    <template #footer>
      <div class="dialog-footer">
        <el-button @click="dialogVisible = false">Cancel</el-button>
        <el-button type="primary" @click="dialogVisible = false"> Confirm </el-button>
      </div>
    </template>
  </el-dialog>
</template>
<style scoped>
.radius {
  height: 20px;
  min-width: 60px;
  border: 2px solid var(--el-color-primary);
  border-radius: var(--el-border-radius-round);
  text-align: center;
  line-height: 20px;
  font-weight: bold;
  padding: 0 10px;
}
.radius:hover {
  background-color: #f0f7ff;
}
.radius:focus {
  background-color: #e5f2ff;
  outline: #e5f2ff solid 3px;
}
</style>
<script lang="ts" setup>
import { useWindowFocus } from '@vueuse/core'
import { useFocus } from '@vueuse/core'
import { ElMessageBox } from 'element-plus'
import { ref, shallowRef, watch } from 'vue'

import { recordHotkey } from '../storage/hotkey'
import { useRecordStore } from '../stores/record'
import { convertToKeyCombo } from '../utils/key'

const dialogVisible = defineModel<boolean>()

const handleClose = (done: () => void) => {
  ElMessageBox.confirm('Are you sure to close this dialog?')
    .then(() => {
      done()
    })
    .catch(() => {
      // catch error
    })
}
interface macro {
  date: string
  name: string
  address: string
}
const currentRow = ref()
const handleCurrentChange = (val: macro | undefined) => {
  currentRow.value = val
}
const handleEdit = (index: number, row: macro) => {
  console.log(index, row)
}
const handleDelete = (index: number, row: macro) => {
  console.log(index, row)
}
const tableData: macro[] = []
const windowFocus = useWindowFocus()
const inputHotkey = shallowRef()
const { focused } = useFocus(inputHotkey)
const recordStore = useRecordStore()

watch(windowFocus, (focused) => {
  if (focused) {
    console.log('window has been focused')
  } else {
    console.log('window has lost focus')
  }
})
watch(focused, (focused) => {
  if (focused) {
    recordStore.keyboardEvents.clear()
    void window.webui.call('beginRecord')
  } else {
    void window.webui.call('stopRecord')
  }
})
watch(recordStore.keyboardEvents, () => {
  const keyCombo = convertToKeyCombo(recordStore.getPressedKeys())
  if (keyCombo[1] !== 0) {
    recordHotkey.set(keyCombo)
  }
})
</script>
