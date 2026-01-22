import { StyleSheet } from 'react-native';
import { COLORS } from './Colors';
import { DIMENSIONS } from './dimensions';

export const commonStyles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: COLORS.BACKGROUND,
  },
  
  centerContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
  
  card: {
    backgroundColor: COLORS.WHITE,
    borderRadius: DIMENSIONS.BORDER_RADIUS_LARGE,
    padding: DIMENSIONS.PADDING_MEDIUM,
    marginBottom: DIMENSIONS.MARGIN_MEDIUM,
    elevation: 2,
    shadowColor: COLORS.BLACK,
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  
  button: {
    backgroundColor: COLORS.PRIMARY,
    paddingHorizontal: DIMENSIONS.PADDING_LARGE,
    paddingVertical: DIMENSIONS.PADDING_MEDIUM,
    borderRadius: DIMENSIONS.BORDER_RADIUS_MEDIUM,
    alignItems: 'center',
    justifyContent: 'center',
  },
  
  buttonText: {
    color: COLORS.WHITE,
    fontSize: DIMENSIONS.FONT_SIZE_MEDIUM,
    fontWeight: 'bold',
  },
  
  title: {
    fontSize: DIMENSIONS.FONT_SIZE_LARGE,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
  },
  
  subtitle: {
    fontSize: DIMENSIONS.FONT_SIZE_MEDIUM,
    color: COLORS.TEXT_SECONDARY,
  },
  
  text: {
    fontSize: DIMENSIONS.FONT_SIZE_MEDIUM,
    color: COLORS.TEXT_PRIMARY,
  },
  
  textSecondary: {
    fontSize: DIMENSIONS.FONT_SIZE_MEDIUM,
    color: COLORS.TEXT_SECONDARY,
  },
  
  row: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  
  spaceBetween: {
    justifyContent: 'space-between',
  },
  
  shadow: {
    elevation: 4,
    shadowColor: COLORS.BLACK,
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.2,
    shadowRadius: 4,
  },
});
