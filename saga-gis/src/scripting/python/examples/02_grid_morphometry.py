#! /usr/bin/env python

import saga_api, sys, os

##########################################
def morphometry(fDEM):
    # ------------------------------------
    # initializations
    
    dem    = saga_api.SG_Get_Data_Manager().Add_Grid(unicode(fDEM))
    if dem == None or dem.is_Valid() == 0:
        print 'ERROR: loading grid [' + fDEM + ']'
        return 0
    
    path   = os.path.split(fDEM)[0]
    if path == '':
        path = './'

    slope  = saga_api.SG_Get_Data_Manager().Add_Grid(dem.Get_System())
    aspect = saga_api.SG_Get_Data_Manager().Add_Grid(dem.Get_System())
    hcurv  = saga_api.SG_Get_Data_Manager().Add_Grid(dem.Get_System())
    vcurv  = saga_api.SG_Get_Data_Manager().Add_Grid(dem.Get_System())
    ccurv  = saga_api.SG_Get_Data_Manager().Add_Grid(dem.Get_System())

    # ------------------------------------
    # 'Slope, Aspect, Curvature'
    
    m      = saga_api.SG_Get_Module_Library_Manager().Get_Module(saga_api.CSG_String('ta_morphometry'), 0)
    p      = m.Get_Parameters()
    p.Get_Grid_System().Assign(dem.Get_System())        # grid module needs to use conformant grid system!
    p(saga_api.CSG_String('ELEVATION')).Set_Value(dem)
    p(saga_api.CSG_String('SLOPE'    )).Set_Value(slope)
    p(saga_api.CSG_String('ASPECT'   )).Set_Value(aspect)
    p(saga_api.CSG_String('C_CROS'   )).Set_Value(hcurv)
    p(saga_api.CSG_String('C_LONG'   )).Set_Value(vcurv)

    if m.Execute() == 0:
        print 'ERROR: executing module [' + m.Get_Name().c_str() + ']'
        return 0

    slope .Save(saga_api.CSG_String(path + '/slope' ))
    aspect.Save(saga_api.CSG_String(path + '/aspect'))
    hcurv .Save(saga_api.CSG_String(path + '/hcurv' ))
    vcurv .Save(saga_api.CSG_String(path + '/vcurv' ))
    
    # ------------------------------------
    # 'Curvature Classification'
    
    m       = saga_api.SG_Get_Module_Library_Manager().Get_Module(saga_api.CSG_String('ta_morphometry'), 4)
    p       = m.Get_Parameters()
    p.Get_Grid_System().Assign(dem.Get_System())        # grid module needs to use conformant grid system!
    p(saga_api.CSG_String('DEM'      )).Set_Value(dem)
    p(saga_api.CSG_String('CLASS'    )).Set_Value(ccurv)
    
    if m.Execute() == 0:
        print 'ERROR: executing module [' + m.Get_Name().c_str() + ']'
        return 0

    ccurv .Save(saga_api.CSG_String(path + '/ccurv' ))
    
    # ------------------------------------
    print 'success'
    
    return 1


##########################################
if __name__ == '__main__':
    print 'Python - Version ' + sys.version
    print saga_api.SAGA_API_Get_Version()
    print

    if len( sys.argv ) != 2:
        print 'Usage: morphometry.py <in: elevation>'
        print '... trying to run with test_data'
        fDEM    = './test.sgrd'
    else:
        fDEM    = sys.argv[1]
        if os.path.split(fDEM)[0] == '':
            fDEM    = './' + fDEM

    saga_api.SG_UI_Msg_Lock(1)
    if os.name == 'nt':    # Windows
        os.environ['PATH'] = os.environ['PATH'] + ';' + os.environ['SAGA_32'] + '/dll'
        saga_api.SG_Get_Module_Library_Manager().Add_Directory(os.environ['SAGA_32' ] + '/modules', 0)
    else:                  # Linux
        saga_api.SG_Get_Module_Library_Manager().Add_Directory(os.environ['SAGA_MLB'], 0)
    saga_api.SG_UI_Msg_Lock(0)

    morphometry(fDEM)
