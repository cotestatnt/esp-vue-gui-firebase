<template>
  <div id="app">
    <login @loggedIn="startListener" />
    <span style="max-with: 100px" v-html="espLogo"></span>
    <gpio ref="gpio" msg="GPIO status list" />
  </div>
</template>

<script>
import login from './components/login'
import gpio from './components/gpio'
import svg from './assets/svg'

export default {
  name: 'App',
  components: {
    gpio,
    login
  },

  data() {
    return {
      espLogo: svg.espLogo,
    };
  },

  methods: {
    startListener() {
      this.$refs.gpio.startListener();
    }
  }


}
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}
body {
  background: #f2f2f2;
}
</style>
