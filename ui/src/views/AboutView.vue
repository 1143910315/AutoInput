<script setup lang="ts">
import { ElementPlus } from '@element-plus/icons-vue'
</script>

<template>
  <div class="about">
    <el-icon><ElementPlus /></el-icon>
    <h1>This is an about page</h1>
  </div>
</template>

<style>
@media (min-width: 1024px) {
  .about {
    min-height: 100vh;
    display: flex;
    align-items: center;
  }
}
</style>
