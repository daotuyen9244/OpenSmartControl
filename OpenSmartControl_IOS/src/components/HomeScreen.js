/**
 * HomeScreen.js
 * Main screen of the massage chair control application
 * 
 * Features:
 * - Display main navigation menu
 * - Show connection status
 * - Provide basic usage instructions
 * - Navigate to sub-screens
 */

import React from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  Image,
  ScrollView,
  Dimensions,
  Platform,
  StatusBar,
  SafeAreaView,
} from 'react-native';
import { useDispatch, useSelector } from 'react-redux';
import { navigateTo } from '../store/navigationSlice';
import { COLORS } from '../styles/Colors';
import { SCREENS } from '../utils/constants';

const { width } = Dimensions.get('window');

const HomeScreen = () => {
  const dispatch = useDispatch();
  const { isConnected } = useSelector(state => state.ble);

  // Configure main menu items
  const menuItems = [
    {
      id: 1,
      title: 'Scan QR Code',
      subtitle: 'Quick connect with QR',
      icon: '📱',
      screen: SCREENS.QR_SCANNER,
      color: COLORS.PRIMARY,
    },
    {
      id: 2,
      title: 'Manual Connect',
      subtitle: 'Find and connect device',
      icon: '🔗',
      screen: SCREENS.MANUAL_CONNECT,
      color: COLORS.SECONDARY,
    },
    {
      id: 3,
      title: 'Control',
      subtitle: 'Control massage chair',
      icon: '🎮',
      screen: SCREENS.CONTROL,
      color: COLORS.WARNING,
      // Allow access even without connection
    },
  ];

  /**
   * Handle menu item press
   * @param {string} screen - Screen name to navigate to
   * @param {boolean} disabled - Disabled state
   */
  const handleMenuPress = (screen, disabled) => {
    if (!disabled) {
      dispatch(navigateTo(screen));
    }
  };

  return (
    <SafeAreaView style={styles.safeArea}>
      <ScrollView style={styles.container} showsVerticalScrollIndicator={false}>
        {/* Header */}
        <View style={styles.header}>
        {/* Logo */}
        <Image 
          source={require('../../icon.png')} 
          style={styles.logo}
          resizeMode="contain"
        />
        <Text style={styles.title}>Massage Chair</Text>
        <Text style={styles.subtitle}>Smart control application</Text>
        
        {/* Display connection status */}
        {isConnected && (
          <View style={styles.statusBadge}>
            <Text style={styles.statusText}>✅ Connected</Text>
          </View>
        )}
      </View>

      {/* Main Menu */}
      <View style={styles.menuContainer}>
        {menuItems.map((item) => (
          <TouchableOpacity
            key={item.id}
            style={[
              styles.menuItem,
              { borderLeftColor: item.color },
              item.disabled && styles.menuItemDisabled
            ]}
            onPress={() => handleMenuPress(item.screen, item.disabled)}
            activeOpacity={0.7}
          >
            {/* Icon */}
            <View style={[styles.menuIcon, { backgroundColor: item.color + '20' }]}>
              <Text style={styles.iconText}>{item.icon}</Text>
            </View>
            
            {/* Content */}
            <View style={styles.menuContent}>
              <Text style={[
                styles.menuTitle,
                item.disabled && styles.menuTitleDisabled
              ]}>
                {item.title}
              </Text>
              <Text style={[
                styles.menuSubtitle,
                item.disabled && styles.menuSubtitleDisabled
              ]}>
                {item.subtitle}
              </Text>
            </View>
            
            {/* Arrow */}
            <View style={styles.menuArrow}>
              <Text style={[
                styles.arrowText,
                item.disabled && styles.arrowTextDisabled
              ]}>
                ›
              </Text>
            </View>
          </TouchableOpacity>
        ))}
      </View>

      {/* Usage Instructions */}
      <View style={styles.infoSection}>
        <Text style={styles.infoTitle}>Usage Instructions</Text>
        
        <View style={styles.infoItem}>
          <Text style={styles.infoNumber}>1</Text>
          <Text style={styles.infoText}>
            Scan QR code on device or connect manually
          </Text>
        </View>
        
        <View style={styles.infoItem}>
          <Text style={styles.infoNumber}>2</Text>
          <Text style={styles.infoText}>
            Wait for successful Bluetooth connection
          </Text>
        </View>
        
        <View style={styles.infoItem}>
          <Text style={styles.infoNumber}>3</Text>
          <Text style={styles.infoText}>
            Control massage chair as desired
          </Text>
        </View>
      </View>
      </ScrollView>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  safeArea: {
    flex: 1,
    backgroundColor: COLORS.WHITE,
  },
  container: {
    flex: 1,
    backgroundColor: COLORS.BACKGROUND,
  },
  header: {
    padding: 20,
    alignItems: 'center',
    backgroundColor: COLORS.WHITE,
    marginBottom: 20,
  },
  logo: {
    width: 120,
    height: 120,
    marginBottom: 16,
  },
  title: {
    fontSize: 28,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 8,
  },
  subtitle: {
    fontSize: 16,
    color: COLORS.TEXT_SECONDARY,
    textAlign: 'center',
  },
  statusBadge: {
    backgroundColor: COLORS.SUCCESS,
    paddingHorizontal: 12,
    paddingVertical: 6,
    borderRadius: 15,
    marginTop: 10,
  },
  statusText: {
    color: COLORS.WHITE,
    fontSize: 12,
    fontWeight: 'bold',
  },
  menuContainer: {
    paddingHorizontal: 20,
    marginBottom: 20,
  },
  menuItem: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: COLORS.WHITE,
    padding: 16,
    marginBottom: 12,
    borderRadius: 12,
    borderLeftWidth: 4,
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  menuItemDisabled: {
    opacity: 0.5,
  },
  menuIcon: {
    width: 50,
    height: 50,
    borderRadius: 25,
    backgroundColor: COLORS.GRAY_LIGHT,
    justifyContent: 'center',
    alignItems: 'center',
    marginRight: 16,
  },
  iconText: {
    fontSize: 24,
  },
  menuContent: {
    flex: 1,
  },
  menuTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 4,
  },
  menuTitleDisabled: {
    color: COLORS.GRAY_MEDIUM,
  },
  menuSubtitle: {
    fontSize: 14,
    color: COLORS.TEXT_SECONDARY,
  },
  menuSubtitleDisabled: {
    color: COLORS.GRAY_MEDIUM,
  },
  menuArrow: {
    width: 30,
    alignItems: 'center',
  },
  arrowText: {
    fontSize: 18,
    color: COLORS.TEXT_SECONDARY,
    fontWeight: 'bold',
  },
  arrowTextDisabled: {
    color: COLORS.GRAY_MEDIUM,
  },
  infoSection: {
    backgroundColor: COLORS.WHITE,
    margin: 20,
    padding: 20,
    borderRadius: 12,
  },
  infoTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 16,
  },
  infoItem: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 12,
  },
  infoNumber: {
    width: 24,
    height: 24,
    borderRadius: 12,
    backgroundColor: COLORS.PRIMARY,
    color: COLORS.WHITE,
    textAlign: 'center',
    lineHeight: 24,
    fontSize: 12,
    fontWeight: 'bold',
    marginRight: 12,
  },
  infoText: {
    flex: 1,
    fontSize: 14,
    color: COLORS.TEXT_SECONDARY,
    lineHeight: 20,
  },
});

export default HomeScreen;
