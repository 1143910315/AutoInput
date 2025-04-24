<template>
  <el-dialog v-model="dialogVisible" title="录制新宏" width="80%" :before-close="handleClose">
    <div class="fl">
      <el-text size="large">开始/结束录制按键</el-text>
      <div class="s1"></div>
      <div class="radius"><el-text type="primary" size="small">F1</el-text></div>
    </div>
    <span>This is a message</span>
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
  width: 60px;
  border: 2px solid var(--el-color-primary);
  border-radius: var(--el-border-radius-round);
  text-align: center;
  line-height: 20px;
  font-weight: bold;
}
</style>
<script lang="ts" setup>
import { ElMessageBox } from 'element-plus'

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
</script>
