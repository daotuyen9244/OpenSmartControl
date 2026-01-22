module.exports = {
  presets: ['@react-native/babel-preset'],
  plugins: [
    [
      'module-resolver',
      {
        root: ['./src'],
        alias: {
          '@': './src',
          '@components': './src/components',
          '@services': './src/services',
          '@store': './src/store',
          '@styles': './src/styles',
        },
      },
    ],
  ],
};
