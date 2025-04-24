<template>
  <el-config-provider :locale="zhCn">
    <el-table
      ref="singleTableRef"
      :data="tableData"
      highlight-current-row
      style="width: 100%"
      @current-change="handleCurrentChange"
    >
      <el-table-column property="address" label="已保存宏" />
      <el-table-column label="操作">
        <template #default="scope">
          <el-button size="small" @click="handleEdit(scope.$index, scope.row)"> 播放 </el-button>
          <el-button size="small" @click="handleEdit(scope.$index, scope.row)"> 编辑 </el-button>
          <el-button size="small" type="danger" @click="handleDelete(scope.$index, scope.row)">
            删除
          </el-button>
        </template>
      </el-table-column>
    </el-table>
    <div class="s2"></div>
    <el-button type="primary" @click="handleAdd">录制新宏</el-button>
    <AddDialog v-model="addDialogVisible"/>
  </el-config-provider>
</template>

<style scoped></style>
<script setup lang="ts">
import type { TableInstance } from 'element-plus'
import zhCn from 'element-plus/es/locale/lang/zh-cn'
import { ref } from 'vue'

import AddDialog from './dialog/AddDialog.vue'
const addDialogVisible = ref(false)

interface User {
  date: string
  name: string
  address: string
}

const currentRow = ref()
const singleTableRef = ref<TableInstance>()
const handleAdd = () => {
  addDialogVisible.value = true
}
const handleCurrentChange = (val: User | undefined) => {
  currentRow.value = val
}
const handleEdit = (index: number, row: User) => {
  console.log(index, row)
}
const handleDelete = (index: number, row: User) => {
  console.log(index, row)
}
const tableData: User[] = []
</script>
