<template>
  <div>
    <div class="fl">
      <el-text class="mx-1">从c++接收的数据：</el-text>
      <el-text class="mx-1 receiveCppData" type="success" @receive-data="receiveData">{{
        data
      }}</el-text>
    </div>
    <div class="s1"></div>
    <div class="fl">
      <el-button type="primary" @click="click">发送点击次数</el-button>
      <div class="s1"></div>
      <el-text class="mx-1">已点击次数：</el-text>
      <el-text class="mx-1" type="success">{{ count }}</el-text>
    </div>
  </div>
</template>

<script lang="ts" setup>
import { ref } from 'vue'

const data = ref('')
const count = ref(0)

// 处理按钮点击事件的函数
const click = () => {
  window.webui.call('clickCount', '点击按钮').then((response) => {
    count.value = parseInt(response, 10)
  })
}

interface MyCustomEventDetail {
  message: string
}

const receiveData = (event: CustomEvent<MyCustomEventDetail>) => {
  data.value = event.detail.message
}
</script>
