import Vue from 'vue'
import App from './App.vue'

import '@/style/bulma.scss';
import '@/style/tooltip.css';

import firebase from 'firebase/app';
const firebaseConfig = {
    apiKey: 'AIzaSyABKehBoLgMYw20B4cwo6v8xVvq5zQi53M',
    authDomain: 'esp-stream-test.firebaseapp.com',
    databaseURL: `https://esp-stream-test-default-rtdb.europe-west1.firebasedatabase.app`,
    projectId: 'esp-stream-test',
    storageBucket: 'esp-stream-test.appspot.com',
    messagingSenderId: '238762683803',
    appId: '1:238762683803:web:2b310f1bc33d861cb2053e',
    measurementId: 'G-8JWHBP5X3B',
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

Vue.config.productionTip = false
new Vue({
  render: h => h(App),
}).$mount('#app')
