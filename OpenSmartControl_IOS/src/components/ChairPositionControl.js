import React, { useState, useRef, useEffect } from 'react';
import {
  View,
  Text,
  TouchableOpacity,
  StyleSheet,
  Alert,
} from 'react-native';
import { useSelector } from 'react-redux';
import bleService from '../services/BleService';
import COLORS from '../styles/Colors';

const ChairPositionControl = () => {
  const [isRecliningUp, setIsRecliningUp] = useState(false);
  const [isRecliningDown, setIsRecliningDown] = useState(false);
  const [isIncliningUp, setIsIncliningUp] = useState(false);
  const [isIncliningDown, setIsIncliningDown] = useState(false);
  const [isRollingUp, setIsRollingUp] = useState(false);
  const [isRollingDown, setIsRollingDown] = useState(false);
  const [isKneading, setIsKneading] = useState(false);
  const [isPercussion, setIsPercussion] = useState(false);
  
  // Get system state from Redux to detect AUTO mode changes
  const systemState = useSelector(state => state.ble?.systemState);
  const isAutoMode = systemState?.isAutoMode || false;
  
  // Refs to store timer intervals (only for press-and-hold buttons)
  const reclineUpTimerRef = useRef(null);
  const reclineDownTimerRef = useRef(null);
  const inclineUpTimerRef = useRef(null);
  const inclineDownTimerRef = useRef(null);
  const rollUpTimerRef = useRef(null);
  const rollDownTimerRef = useRef(null);
  
  // Reset M KNEADING and M PERCUSSION state when switching to AUTO mode
  useEffect(() => {
    if (isAutoMode) {
      // When switching to AUTO mode, reset manual control states
      if (isKneading || isPercussion) {
        console.log('🔄 AUTO mode activated - Resetting M KNEADING and M PERCUSSION states');
        setIsKneading(false);
        setIsPercussion(false);
      }
    }
  }, [isAutoMode]);

  const handlePressIn = (action, setState, timerRef, direction, commandType) => {
    console.log(`=== PRESS IN ===`);
    console.log(`Action: ${action.name}`);
    console.log(`Direction: ${direction}`);
    console.log(`Command Type: ${commandType}`);
    
    // Clear any existing timer
    if (timerRef.current) {
      clearInterval(timerRef.current);
      timerRef.current = null;
    }
    
    // Set state to true
    setState(true);
    console.log('Set state to true - PRESSED');
    
    // Send PRESSED command (direction 1 or 2)
    console.log(`Sending PRESSED command: ${commandType}:${direction}`);
    action.call(bleService, true, direction).catch(error => {
      console.error('Press command error:', error);
      setState(false);
    });
  };

  const handlePressOut = (action, setState, timerRef, commandType) => {
    console.log(`=== PRESS OUT ===`);
    console.log(`Action: ${action.name}`);
    console.log(`Command Type: ${commandType}`);
    
    // Clear timer
    if (timerRef.current) {
      clearInterval(timerRef.current);
      timerRef.current = null;
    }
    
    // Set state to false
    setState(false);
    console.log('Set state to false - RELEASED');
    
    // Send RELEASED command (direction 0 = stop)
    console.log(`Sending RELEASED command: ${commandType}:0`);
    action.call(bleService, false, 0).catch(error => {
      console.error('Release command error:', error);
    });
  };

  // Cleanup on component unmount
  React.useEffect(() => {
    return () => {
      // Clear any remaining timers if they exist (only for press-and-hold buttons)
      if (reclineUpTimerRef.current) clearInterval(reclineUpTimerRef.current);
      if (reclineDownTimerRef.current) clearInterval(reclineDownTimerRef.current);
      if (inclineUpTimerRef.current) clearInterval(inclineUpTimerRef.current);
      if (inclineDownTimerRef.current) clearInterval(inclineDownTimerRef.current);
      if (rollUpTimerRef.current) clearInterval(rollUpTimerRef.current);
      if (rollDownTimerRef.current) clearInterval(rollDownTimerRef.current);
    };
  }, []);

  return (
    <View style={styles.controlGrid}>
      {/* Row 1: RECLINE and INCLINE */}
      <View style={styles.row}>
        <TouchableOpacity
          style={[styles.button, isRecliningDown && styles.buttonPressed]}
          onPressIn={() => handlePressIn(bleService.controlRecline, setIsRecliningDown, reclineDownTimerRef, 1, 'RECLINE')}
          onPressOut={() => handlePressOut(bleService.controlRecline, setIsRecliningDown, reclineDownTimerRef, 'RECLINE')}
          activeOpacity={0.8}
        >
          <Text style={[styles.buttonText, isRecliningDown && styles.buttonTextPressed]}>
            M RECLINE
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.button, isRecliningUp && styles.buttonPressed]}
          onPressIn={() => handlePressIn(bleService.controlIncline, setIsRecliningUp, reclineUpTimerRef, 2, 'INCLINE')}
          onPressOut={() => handlePressOut(bleService.controlIncline, setIsRecliningUp, reclineUpTimerRef, 'INCLINE')}
          activeOpacity={0.8}
        >
          <Text style={[styles.buttonText, isRecliningUp && styles.buttonTextPressed]}>
            M INCLINE
          </Text>
        </TouchableOpacity>
      </View>

      {/* Row 2: BACKWARD and FORWARD */}
      <View style={styles.row}>
        <TouchableOpacity
          style={[styles.button, isIncliningDown && styles.buttonPressed]}
          onPressIn={() => handlePressIn(bleService.controlBackward, setIsIncliningDown, inclineDownTimerRef, 1, 'BACKWARD')}
          onPressOut={() => handlePressOut(bleService.controlBackward, setIsIncliningDown, inclineDownTimerRef, 'BACKWARD')}
          activeOpacity={0.8}
        >
          <Text style={[styles.buttonText, isIncliningDown && styles.buttonTextPressed]}>
            M BACKWARD
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.button, isIncliningUp && styles.buttonPressed]}
          onPressIn={() => handlePressIn(bleService.controlForward, setIsIncliningUp, inclineUpTimerRef, 2, 'FORWARD')}
          onPressOut={() => handlePressOut(bleService.controlForward, setIsIncliningUp, inclineUpTimerRef, 'FORWARD')}
          activeOpacity={0.8}
        >
          <Text style={[styles.buttonText, isIncliningUp && styles.buttonTextPressed]}>
            M FORWARD
          </Text>
        </TouchableOpacity>
      </View>

      {/* Row 3: ROLL UP and ROLL DOWN */}
      <View style={styles.row}>
        <TouchableOpacity
          style={[styles.button, isRollingUp && styles.buttonPressed]}
          onPressIn={() => handlePressIn(bleService.controlRollUp, setIsRollingUp, rollUpTimerRef, 1, 'ROLL_UP')}
          onPressOut={() => handlePressOut(bleService.controlRollUp, setIsRollingUp, rollUpTimerRef, 'ROLL_UP')}
          activeOpacity={0.8}
        >
          <Text style={[styles.buttonText, isRollingUp && styles.buttonTextPressed]}>
            M ROLL UP
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.button, isRollingDown && styles.buttonPressed]}
          onPressIn={() => handlePressIn(bleService.controlRollDown, setIsRollingDown, rollDownTimerRef, 2, 'ROLL_DOWN')}
          onPressOut={() => handlePressOut(bleService.controlRollDown, setIsRollingDown, rollDownTimerRef, 'ROLL_DOWN')}
          activeOpacity={0.8}
        >
          <Text style={[styles.buttonText, isRollingDown && styles.buttonTextPressed]}>
            M ROLL DOWN
          </Text>
        </TouchableOpacity>
      </View>

      {/* Row 4: KNEADING and PERCUSSION */}
      <View style={styles.row}>
        <TouchableOpacity
          style={[styles.button, isKneading && styles.buttonPressed]}
          onPress={async () => {
            const newState = !isKneading;
            setIsKneading(newState);
            try {
              await bleService.controlKneading(newState, 0);
            } catch (error) {
              console.error('Kneading toggle error:', error);
              setIsKneading(!newState); // Revert state on error
            }
          }}
          activeOpacity={0.8}
        >
          <Text style={[styles.buttonText, isKneading && styles.buttonTextPressed]}>
            M KNEADING
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.button, isPercussion && styles.buttonPressed]}
          onPress={async () => {
            const newState = !isPercussion;
            setIsPercussion(newState);
            try {
              await bleService.controlPercussion(newState, 0);
            } catch (error) {
              console.error('Percussion toggle error:', error);
              setIsPercussion(!newState); // Revert state on error
            }
          }}
          activeOpacity={0.8}
        >
          <Text style={[styles.buttonText, isPercussion && styles.buttonTextPressed]}>
            M PERCUSSION
          </Text>
        </TouchableOpacity>
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    padding: 18,
    backgroundColor: COLORS.BACKGROUND,
  },
  title: {
    fontSize: 22,
    fontWeight: 'bold',
    color: COLORS.PRIMARY,
    textAlign: 'center',
    marginBottom: 8,
  },
  subtitle: {
    fontSize: 10,
    color: COLORS.TEXT_SECONDARY,
    textAlign: 'center',
    marginBottom: 8,
  },
  controlGrid: {
    gap: 5,
  },
  row: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    gap: 10,
  },
  button: {
    width: '48%',
    padding: 12,
    borderRadius: 8,
    borderWidth: 2,
    borderColor: COLORS.GRAY_MEDIUM,
    backgroundColor: COLORS.WHITE,
    justifyContent: 'center',
    alignItems: 'center',
    elevation: 3,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    marginBottom: 10,
  },
  buttonPressed: {
    backgroundColor: COLORS.PRIMARY,
    elevation: 1,
    shadowOpacity: 0.05,
  },
  buttonText: {
    fontSize: 12,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    textAlign: 'center',
  },
  buttonTextPressed: {
    color: COLORS.WHITE,
  },
  
});

export default ChairPositionControl;
