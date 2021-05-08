<template>
  <div>
    <form  v-if="!loggedIn" @submit.prevent="login">
      <p>Login as authorized user for Firebase project</p>
      <input
        type="email"
        placeholder="Email address..."
        v-model="email"
      />
      <input
        type="password"
        placeholder="password..."
        v-model="password"
      />
      <button type="submit">Login</button>

    </form>
    <button v-if="loggedIn" @click="logout">Logout</button>

  </div>
</template>

<script>

import firebase from 'firebase/app';
import 'firebase/auth';

export default {
  name: 'login',
  data() {
    return {
      loggedIn: false,
      email: '',
      password: '',
    };
  },

  methods: {
    login() {
      firebase
        .auth()
        .signInWithEmailAndPassword(this.email, this.password)
        .then(() => {
            console.log('Successfully logged in');
            this.loggedIn = true;
            this.$emit("loggedIn");
        })
        .catch(error => {
            alert(error.message);
        });
    },

    logout() {
      firebase
        .auth()
        .signOut()
        .then(() => {
          alert('Successfully logged out');
          this.loggedIn = false;
        })
        .catch(error => {
          alert(error.message);
        });
    },

  },
};
</script>
